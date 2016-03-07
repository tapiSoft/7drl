#pragma once

#include "7drl.hpp"
#include "items.hpp"

#include <entityx/entityx.h>

using namespace entityx;

enum RenderState {
	RenderGame,
	RenderInventory,
};

struct GameState
{
	RenderState render;
	EntityX ex;
	Entity playerentity;
	Level currentLevel;
	GameState();

	// Returns false if game should exit
	bool handleInput(TCOD_key_t key);
	void movePlayer(int16_t dx, int16_t dy);
	void toggleInventory();
	void renderState();
	void newLevel();
};
