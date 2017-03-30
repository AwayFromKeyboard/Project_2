#include "Defs.h"
#include "Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "SceneTest.h"
#include <sstream>
#include "j1Entity.h"
#include "Hero.h"
#include "GameObject.h"
#include "j1Collisions.h"

SceneTest::SceneTest()
{
}

SceneTest::~SceneTest()
{
}

bool SceneTest::Start()
{
	if (App->map->Load("iso_walk.tmx") == true)
	{
		int w, h;
		uchar* data = NULL;
		if (App->map->CreateWalkabilityMap(w, h, &data))
			App->pathfinding->SetMap(w, h, data);

		RELEASE_ARRAY(data);
	}
	debug_tex = App->tex->LoadTexture("maps/path2.png");

	App->collisions->UpdateQuadtree();

	cursor_window = (UI_Window*)App->gui->UI_CreateWin(iPoint(0, 0), 37, 40, 100, true);
	cursor_r = { 1, 1, 37, 40 };
	cursor = (UI_Image*)cursor_window->CreateImage(iPoint(0, 0), cursor_r, true);

	general_ui_window = (UI_Window*)App->gui->UI_CreateWin(iPoint(0, 0), App->win->_GetWindowSize().x, App->win->_GetWindowSize().y, 3);
	ui_r = { 1, 84, 800, 600 };
	general_ui_image = (UI_Image*)general_ui_window->CreateImage(iPoint(0, 0), ui_r);
	
	InitCameraMovement();

	troop = (Hero*)App->entity->CreateEntity(player);
	fPoint pos(App->map->MapToWorld(12, 0).x, App->map->MapToWorld(12, 0).y);
	troop->game_object->SetPos(pos);

	troop2 = (Hero*)App->entity->CreateEntity(player);
	fPoint pos2(App->map->MapToWorld(13, 1).x, App->map->MapToWorld(13, 1).y);
	troop2->game_object->SetPos(pos2);

	gold = 1000;
	gold_txt = (UI_Text*)general_ui_window->CreateText({ 500, 25 }, App->font->default);

	SDL_ShowCursor(0);
	return true;
}

bool SceneTest::PreUpdate()
{
	int x, y;
	App->input->GetMousePosition(x, y);
	iPoint p = App->render->ScreenToWorld(x, y);
	p = App->map->WorldToMap(p.x, p.y);

CheckUnitCreation(p);
  
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == key_down)
	{
		//troop->SetPath(App->pathfinding->CreatePath(App->map->WorldToMapPoint(troop->game_object->GetPos()), p));
		if (App->map->WorldToMapPoint(troop2->GetGameObject()->GetPos()) == p && troop2->life > 0)
		{
			troop->state = unit_attack;
			troop->SetAttackingUnit(troop2);
		}

	}
	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == key_down) {
		troop->SetPath(App->pathfinding->CreatePath(App->map->WorldToMapPoint(troop->game_object->GetPos()), p));
	}

	return true;
}

bool SceneTest::Update(float dt)
{
	iPoint mouse;
	App->input->GetMouseWorld(mouse.x, mouse.y);
	
	UpdateCameraMovement();

	App->map->Draw();
	cursor->Set(iPoint(mouse.x, mouse.y), cursor_r);

	

	if (troop->path.size() > 0)
	{
		troop->state = unit_move;
	}


	for (uint i = 0; i < troop->path.size(); i++)
	{
		iPoint pos = App->map->MapToWorld(troop->path.at(i).x, troop->path.at(i).y);
		App->render->Blit(debug_tex, pos.x, pos.y);
	}
	

	
	return true;
}

bool SceneTest::PostUpdate()
{
	return true;
}

bool SceneTest::CleanUp()
{
	/*if (App->scene->GetCurrentScene() != App->scene->scene_test)
	{
		App->gui->DeleteElement(cursor);
		App->gui->DeleteElement(general_ui_window);
		App->gui->DeleteElement(gold_txt);
	}*/

	return true;
}

bool SceneTest::Load(pugi::xml_node &)
{
	return true;
}

bool SceneTest::Save(pugi::xml_node &) const
{
	return true;
}

void SceneTest::OnColl(Collider* col1, Collider* col2)
{
	if (col1 != nullptr && (col2->type == COLLIDER_UNIT))
	{
		troop->SetPath(App->pathfinding->CreatePath(App->map->WorldToMapPoint(troop->game_object->GetPos()), App->map->WorldToMapPoint(troop->game_object->GetPos())));
	}
}

void SceneTest::CheckUnitCreation(iPoint p)
{
	std::stringstream oss;
	oss << "Gold: " << gold;
	std::string txt = oss.str();
	gold_txt->SetText(txt);

	if (App->input->GetKey(SDL_SCANCODE_C) == key_down && gold >= TROOP_PRICE)
	{
		gold -= TROOP_PRICE;
		troop = (Hero*)App->entity->CreateEntity(player);
		fPoint pos(App->map->MapToWorld(p.x + TROOP_OFFSET, p.y).x, App->map->MapToWorld(p.x + TROOP_OFFSET, p.y).y);
		troop->game_object->SetPos(pos);
	}
}