#include "Defs.h"
#include "Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1FileSystem.h"
#include "j1Textures.h"
#include "j1Map.h"
#include <math.h>
#include "j1Window.h"
#include "Entity.h"
#include "j1Entity.h"
#include "j1App.h"
#include "Player.h"
#include "Functions.h"
#include <sstream> 

j1Map::j1Map() : j1Module(), map_loaded(false)
{
	name = "map";
}

// Destructor
j1Map::~j1Map()
{}

// Called before render is available
bool j1Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	folder = config.child("folder").child_value();
	draw_margin = config.child("draw_margin").attribute("value").as_int();

	offset = 0;

	fit_square.x = 0; fit_square.y = 0;
	uint w, h;
	App->win->GetWindowSize(w, h);
	fit_square.w = w;
	fit_square.h = h;

	return ret;
}

void j1Map::Draw()
{
	if (map_loaded == false)
		return;
	fit_square = { -(App->render->camera.x - XOFFSET), -(App->render->camera.y - YOFFSET), App->render->camera.w, App->render->camera.h };
	for (std::list<MapLayer*>::iterator item = data.layers.begin(); item != data.layers.end(); item++)
	{
		MapLayer* layer = *item;

		if(!App->debug_mode && (layer->properties.Get("Nodraw") != 0 || layer->name == "entities"))
			continue;

		int x_ini, x_end;
		TilesToDraw_x(x_ini, x_end, *item);
		int count = 0;
		for (int x = x_ini; x < x_end; ++x)
		{
			int y_ini, y_end;
			TilesToDraw_y(count, x, x_end, y_ini, y_end, *item);

			for (int y = y_ini; y < y_end; ++y)
			{
				int tile_id = layer->Get(x, y);
				if (tile_id > 0)
				{
					TileSet* tileset = GetTilesetFromTileId(tile_id);

					SDL_Rect r = tileset->GetTileRect(tile_id);
					iPoint pos = MapToWorld(x, y);
					//magic numbers +30 to fix
					if(layer->name == "Nature")
						App->render->Blit(tileset->texture, pos.x - 12, pos.y - 140, &r);
					else if (layer->name == "Buildings")
						App->render->Blit(tileset->texture, pos.x, pos.y - 96, &r);
					else if (layer->name == "Towers")
						App->render->Blit(tileset->texture, pos.x + 10, pos.y - 80, &r);
					else if (layer->name == "Fortress")
						App->render->Blit(tileset->texture, pos.x, pos.y - 198, &r);
					else if (layer->name == "Big House")
						App->render->Blit(tileset->texture, pos.x - 12, pos.y - 126, &r);
					else
						App->render->Blit(tileset->texture, pos.x, pos.y + 16, &r);
				}
			}
			count++;
		}
	}
}

int Properties::Get(const char* value, int default_value) const
{
	for (std::list<Property*>::const_iterator item = list.begin(); item != list.end(); ++item)
	{
		if ((*item)->name == value)
			return (*item)->value;
	}

	return default_value;
}

TileSet* j1Map::GetTilesetFromTileId(int id) const
{
	std::list<TileSet*>::const_iterator item = data.tilesets.begin();
	TileSet* set = *item;

	while (item != data.tilesets.end())
	{
		if (id < (*item)->firstgid)
		{
			set = *(--item);
			break;
		}
		set = *item;
		item++;
	}

	return set;
}

iPoint j1Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	if (data.type == maptype_orthogonal)
	{
		ret.x = x * data.tile_width;
		ret.y = y * data.tile_height;
	}
	else if (data.type == maptype_isometric)
	{
		ret.x = (x - y) * (data.tile_width * 0.5f);
		ret.y = (x + y) * (data.tile_height * 0.5f);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

fPoint j1Map::FMapToWorld(int x, int y) const
{
	fPoint ret;

	if (data.type == maptype_orthogonal)
	{
		ret.x = x * data.tile_width;
		ret.y = y * data.tile_height;
	}
	else if (data.type == maptype_isometric)
	{
		ret.x = (x - y) * (data.tile_width * 0.5f);
		ret.y = (x + y) * (data.tile_height * 0.5f);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

iPoint j1Map::WorldToMap(int x, int y) const
{
	iPoint ret(0, 0);

	if (data.type == maptype_orthogonal)
	{
		ret.x = x / data.tile_width;
		ret.y = y / data.tile_height;
	}
	else if (data.type == maptype_isometric)
	{

		float half_width = data.tile_width * 0.5f;
		float half_height = data.tile_height * 0.5f;
		ret.x = int((x / half_width + y / half_height) / 2) - 1;
		ret.y = int((y / half_height - (x / half_width)) / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

iPoint j1Map::WorldToMapPoint(iPoint position) const
{
	iPoint ret(0, 0);

	if (data.type == maptype_orthogonal)
	{
		ret.x = position.x / data.tile_width;
		ret.y = position.y / data.tile_height;
	}
	else if (data.type == maptype_isometric)
	{

		float half_width = data.tile_width * 0.5f;
		float half_height = data.tile_height * 0.5f;
		ret.x = int((position.x / half_width + position.y / half_height) / 2) - 1;
		ret.y = int((position.y / half_height - (position.x / half_width)) / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = position.x; ret.y = position.y;
	}

	return ret;
}

iPoint j1Map::MapToWorldPoint(iPoint position) const
{
	iPoint ret;

	if (data.type == maptype_orthogonal)
	{
		ret.x = position.x * data.tile_width;
		ret.y = position.y * data.tile_height;
	}
	else if (data.type == maptype_isometric)
	{
		ret.x = (position.x - position.y) * (data.tile_width * 0.5f);
		ret.y = (position.x + position.y) * (data.tile_height * 0.5f);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = position.x; ret.y = position.y;
	}

	return ret;
}

SDL_Rect TileSet::GetTileRect(int id) const
{
	int relative_id = id - firstgid;
	SDL_Rect rect;
	rect.w = tile_width;
	rect.h = tile_height;
	rect.x = margin + ((rect.w + spacing) * (relative_id % num_tiles_width));
	rect.y = margin + ((rect.h + spacing) * (relative_id / num_tiles_width));
	return rect;
}

// Called before quitting
bool j1Map::CleanUp()
{
	LOG("Unloading map");

	// Remove all tilesets
	for (std::list<TileSet*>::iterator it = data.tilesets.begin(); it != data.tilesets.end(); ++it) 
		RELEASE(*it);
	data.tilesets.clear();

	// Remove all layers
	for (std::list<MapLayer*>::iterator it = data.layers.begin(); it != data.layers.end(); ++it)
		RELEASE(*it);
	data.layers.clear();

	// Clean up the pugui tree
	map_file.reset();

	return true;
}

// Load new map
bool j1Map::Load(const char* file_name)
{
	bool ret = true;
	std::stringstream oss;
	oss << folder.c_str() << file_name;
	std::string tmp = oss.str();

	char* buf;
	int size = App->fs->Load(tmp.c_str(), &buf);
	pugi::xml_parse_result result = map_file.load_buffer(buf, size);

	RELEASE(buf);

	if (result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", file_name, result.description());
		ret = false;
	}

	// Load general info ----------------------------------------------
	if (ret == true)
	{
		ret = LoadMap();
	}

	// Load all tilesets info ----------------------------------------------
	pugi::xml_node tileset;
	for (tileset = map_file.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if (ret == true)
		{
			ret = LoadTilesetDetails(tileset, set);
		}

		if (ret == true)
		{
			ret = LoadTilesetImage(tileset, set);
		}

		data.tilesets.push_back(set);
	}

	// Load layer info ----------------------------------------------
	pugi::xml_node layer;
	for (layer = map_file.child("map").child("layer"); layer && ret; layer = layer.next_sibling("layer"))
	{
		MapLayer* lay = new MapLayer();

		ret = LoadLayer(layer, lay);

		if (ret == true)
			data.layers.push_back(lay);
	}

	// ret = LoadResources(map_file.child("map"));
	if (ret == true)
	{
		LOG("Successfully parsed map XML file: %s", file_name);
		LOG("width: %d height: %d", data.width, data.height);
		LOG("tile_width: %d tile_height: %d", data.tile_width, data.tile_height);

		std::list<TileSet*>::iterator item = data.tilesets.begin();
		while (item != data.tilesets.end())
		{
			TileSet* s = *item;
			LOG("Tileset ----");
			LOG("name: %s firstgid: %d", s->name.c_str(), s->firstgid);
			LOG("tile width: %d tile height: %d", s->tile_width, s->tile_height);
			LOG("spacing: %d margin: %d", s->spacing, s->margin);
			item++;
		}

		std::list<MapLayer*>::iterator item_layer = data.layers.begin();
		while (item_layer != data.layers.end())
		{
			MapLayer* l = *item_layer;
			LOG("Layer ----");
			LOG("name: %s", l->name.c_str());
			LOG("tile width: %d tile height: %d", l->width, l->height);
			item_layer++;
		}
	}

	map_loaded = ret;

	// Load the matrix -----------------
	vector<void*> tmp_vec;
	for (int j = 0; j < data.height; ++j) 
		tmp_vec.push_back(nullptr);

	for (int i = 0; i < data.width; ++i)
		entity_matrix.push_back(tmp_vec);

	return ret;
}

// Load map general properties
bool j1Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = map_file.child("map");

	if (map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		data.width = map.attribute("width").as_int();
		data.height = map.attribute("height").as_int();
		data.tile_width = map.attribute("tilewidth").as_int();
		data.tile_height = map.attribute("tileheight").as_int();
		std::string bg_color(map.attribute("backgroundcolor").as_string());

		data.background_color.r = 0;
		data.background_color.g = 0;
		data.background_color.b = 0;
		data.background_color.a = 0;

		if (bg_color.length() > 0)
		{
			std::string red, green, blue;
			red = bg_color.substr(1, 2);
			green = bg_color.substr(3, 2);
			blue = bg_color.substr(5, 2);

			int v = 0;

			sscanf_s(red.c_str(), "%x", &v);
			if (v >= 0 && v <= 255) data.background_color.r = v;

			sscanf_s(green.c_str(), "%x", &v);
			if (v >= 0 && v <= 255) data.background_color.g = v;

			sscanf_s(blue.c_str(), "%x", &v);
			if (v >= 0 && v <= 255) data.background_color.b = v;
		}

		std::string orientation(map.attribute("orientation").as_string());

		if (orientation == "orthogonal")
		{
			data.type = maptype_orthogonal;
		}
		else if (orientation == "isometric")
		{
			data.type = maptype_isometric;
		}
		else if (orientation == "staggered")
		{
			data.type = maptype_staggered;
		}
		else
		{
			data.type = maptype_unknown;
		}
	}

	return ret;
}

bool j1Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	set->name = (tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tile_width = tileset_node.attribute("tilewidth").as_int();
	set->tile_height = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if (offset != NULL)
	{
		set->offset_x = offset.attribute("x").as_int();
		set->offset_y = offset.attribute("y").as_int();
	}
	else
	{
		set->offset_x = 0;
		set->offset_y = 0;
	}

	return ret;
}

bool j1Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if (image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		set->texture = App->tex->LoadTexture(PATH(folder.c_str(), image.attribute("source").as_string()));
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);
		set->tex_width = image.attribute("width").as_int();

		if (set->tex_width <= 0)
		{
			set->tex_width = w;
		}

		set->tex_height = image.attribute("height").as_int();

		if (set->tex_height <= 0)
		{
			set->tex_height = h;
		}

		set->num_tiles_width = set->tex_width / set->tile_width;
		set->num_tiles_height = set->tex_height / set->tile_height;
	}

	return ret;
}

bool j1Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();
	LoadProperties(node, layer->properties);
	pugi::xml_node layer_data = node.child("data");

	if (layer_data == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'layer/data' tag.");
		ret = false;
		RELEASE(layer);
	}
	else
	{
		layer->data = new uint[layer->width*layer->height];
		memset(layer->data, 0, layer->width*layer->height);

		int i = 0;
		for (pugi::xml_node tile = layer_data.child("tile"); tile; tile = tile.next_sibling("tile"))
		{
			layer->data[i++] = tile.attribute("gid").as_int(0);
		}
	}

	return ret;
}

// Load a group of properties from a node and fill a list with it
bool j1Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	bool ret = false;

	pugi::xml_node data = node.child("properties");

	if (data != NULL)
	{
		pugi::xml_node prop;

		for (prop = data.child("property"); prop; prop = prop.next_sibling("property"))
		{
			Properties::Property* p = new Properties::Property();

			p->name = prop.attribute("name").as_string();
			p->value = prop.attribute("value").as_int();

			properties.list.push_back(p);
		}
	}

	return ret;
}

bool j1Map::LoadResources(pugi::xml_node & node)
{
	bool ret = true;
	//data.mapWidth = data.width * data.tile_width;
	//data.mapHeight = data.height * data.tile_height;

	/*pugi::xml_node resourceNode;

	for (resourceNode = node.child("objectgroup"); resourceNode; resourceNode = resourceNode.next_sibling("objectgroup"))
	{
		pugi::xml_node prop;

		for (prop = resourceNode.child("object"); prop; prop = prop.next_sibling("object"))
		{
			
			string name = prop.attribute("name").as_string();
			Resource* resource = App->entityManager->CreateResource(prop.attribute("x").as_int(), prop.attribute("y").as_int(), (resourceType)type, 0);
			
			data.PropResources.push_back(resource);
			
		}
	}
	*/
	return ret;
}

void j1Map::TilesToDraw_x(int & x_ini, int & x_end, MapLayer * layer) const
{
	iPoint ini = WorldToMap(fit_square.x, fit_square.y);
	iPoint end = WorldToMap(fit_square.x + fit_square.w, fit_square.y + fit_square.h);
	switch (data.type)
	{
	case maptype_unknown:
		break;
	case maptype_orthogonal:
		x_ini = (ini.x - draw_margin > 0) ? (ini.x - draw_margin) : 0;
		x_end = (end.x + draw_margin < layer->width) ? (end.x + draw_margin) : layer->width;
		break;
	case maptype_isometric:
		x_ini = (ini.x - draw_margin > 0) ? (ini.x - draw_margin) : 0;
		x_end = (end.x + draw_margin < layer->width) ? (end.x + draw_margin) : layer->width;
		break;
	case maptype_staggered:
		break;
	default:
		break;
	}
}

void j1Map::TilesToDraw_y(int count, int x, int x_end, int & y_ini, int & y_end, MapLayer * layer) const
{
	iPoint top_right = WorldToMap(fit_square.x + fit_square.w, fit_square.y);
	iPoint top_left = WorldToMap(fit_square.x, fit_square.y);
	iPoint bot_right = WorldToMap(fit_square.x + fit_square.w, fit_square.y + fit_square.h);
	iPoint bot_left = WorldToMap(fit_square.x, fit_square.y + fit_square.h);
	switch (data.type)
	{
	case maptype_unknown:
		break;
	case maptype_orthogonal:
		y_ini = (top_left.y - draw_margin > 0) ? (top_left.y - draw_margin) : 0;
		y_end = (bot_right.y + draw_margin < layer->height) ? bot_right.y + draw_margin : layer->height;
		break;
	case maptype_isometric: {
		float screen_prop = (float)fit_square.w / (float)fit_square.h;
		int diag1 = bot_left.x, diag2 = top_right.x;
		if (x <= diag1) {
			y_ini = top_left.y - (count + 1);
			y_end = top_left.y + count + 2;
			if (top_left.x <= 0) {
				y_ini += (top_left.x - 3);
				y_end -= (top_left.x - 3);
			}
		}
		else if (x > diag1 && x < diag2) {
			int diff = x - diag1;
			y_ini = top_left.y - count - diff;
			y_end = top_left.y + diff;
		}
		else {
			int rev_count = bot_right.x - x;
			y_ini = bot_right.y - rev_count;
			y_end = bot_right.y + (rev_count + 1);
		}

		if (y_ini < 0) y_ini = 0;
		if (y_end > layer->height) y_end = layer->height;

		break;
	}
	case maptype_staggered:
		break;
	default:
		break;
	}
}

bool j1Map::CreateWalkabilityMap(int& width, int& height, uchar** buffer) const
{
	bool ret = false;
	std::list<MapLayer*>::const_iterator item;
	item = data.layers.begin();

	for (; item != data.layers.end(); item++)
	{
		MapLayer* layer = *item;

		if (layer->properties.Get("Navigation", 0) == 0)
			continue;

		uchar* map = new uchar[layer->width*layer->height];
		memset(map, 1, layer->width*layer->height);

		for (int y = 0; y < data.height; ++y)
		{
			for (int x = 0; x < data.width; ++x)
			{
				int i = (y*layer->width) + x;

				int tile_id = layer->Get(x, y);
				TileSet* tileset = (tile_id > 0) ? GetTilesetFromTileId(tile_id) : NULL;

				if (tileset != NULL)
				{
					map[i] = (tile_id - tileset->firstgid) > 0 ? 0 : 1;
					/*TileType* ts = tileset->GetTileType(tile_id);
					if(ts != NULL)
					{
					map[i] = ts->properties.Get("walkable", 1);
					}*/
				}
			}
		}

		*buffer = map;
		width = data.width;
		height = data.height;
		ret = true;

		break;
	}

	return ret;
}

void j1Map::GetEntitiesSpawn() const
{
	for (std::list<MapLayer*>::const_iterator item = data.layers.begin(); item != data.layers.end(); item++)
	{
		MapLayer* layer = *item;

		if (layer->name != "entities")
			continue;

		for (int y = 0; y < data.height; ++y)
		{
			for (int x = 0; x < data.width; ++x)
			{
				int id = layer->Get(x, y);
				if (id != 0)
				{
					TileSet* tileset = (id > 0) ? GetTilesetFromTileId(id) : NULL;
					if (tileset != NULL)
					{
						int relative_id = id - tileset->firstgid;
						switch (id)
						{
						case 27: // Hero
						{
							Entity* player_unit = App->entity->CreateEntity(hero, player, App->map->MapToWorld(x + 1, y));
							App->player->SetHero((Hero*)player_unit);
						}
						break;
						
						case 28: // Barbarian Enemy
						{
							Entity* barb_enemy = App->entity->CreateEntity(barbarian, enemy, App->map->MapToWorld(x + 1, y));
						}
						break;
						
						case 29: // NPC
						{
							Entity* barb_npc = App->entity->CreateEntity(barbarian, npc, App->map->MapToWorld(x + 1, y));
						}
						break;
						
						case 30: // Object
						{
							Entity* object_entity = App->entity->CreateEntity(provisions, object, App->map->MapToWorld(x + 1, y));
						}
						break;

						case 31: // Basic Building Ally
						{
							Entity* basicbuilding_ally = App->entity->CreateBuildingEntity(basic_building, ally_building, App->map->MapToWorld(x, y - 1), RandomGenerate(1, 3));
						}
						break;

						case 32: // Basic Building Enemy
						{
							Entity* basicbuilding_enemy = App->entity->CreateBuildingEntity(basic_building, enemy_building, App->map->MapToWorld(x, y - 1), RandomGenerate(1, 3));
						}
						break;

						case 33: // Swordsman Enemy
						{
							Entity* swordsman_enemy = App->entity->CreateEntity(swordsman, enemy, App->map->MapToWorld(x + 1, y));
						}
						break;
					}
					}
				}		
			}
		}
	}
}