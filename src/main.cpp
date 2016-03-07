#include <iostream>

#include <libtcod.hpp>
#include <console.hpp>

#include "7drl.hpp"
#include "items.hpp"
#include "config.hpp"
#include "util/cpptoml.h"

#include <entityx/entityx.h>

using namespace entityx;

std::unique_ptr<Config> GLOBALCONFIG = nullptr;

enum RenderState {
	RenderGame,
	RenderInventory,
};

struct GameState
{
	RenderState render;
	EntityX ex;
	Entity playerentity;
	GameState() : render(RenderGame) {
		playerentity = ex.entities.create();
		playerentity.assign<Position>(40, 25);
		playerentity.assign<Model>('@', TCODColor::white);

		playerentity.assign<Inventory>(Inventory {
			{itemList[0]},
		});
	}

	// Returns false if game should exit
	bool handleInput(TCOD_key_t key)
	{
		if(key.pressed)
		{
			switch (key.vk) {
			// TODO: Do we want to support arrow keys?
			/*
			case TCODK_LEFT:
				movePlayer(-1, 0);
				break;
			case TCODK_RIGHT:
				movePlayer(1, 0);
				break;
			case TCODK_UP:
				movePlayer(0, -1);
				break;
			case TCODK_DOWN:
				movePlayer(0, 1);
				break;
			*/
			case TCODK_TAB:
				if(!GLOBALCONFIG->keybindings.inventory) {
					toggleInventory();
				}
				break;
			case TCODK_SPACE:
				if(!GLOBALCONFIG->keybindings.idle) {
					break;
				}
				break;
			case TCODK_ESCAPE:
				return false;
			case TCODK_CHAR:
				if(this->render == RenderGame)
				{
					if(key.c == GLOBALCONFIG->keybindings.upleft) movePlayer(-1, -1);
					else if(key.c == GLOBALCONFIG->keybindings.up) movePlayer(0, -1);
					else if(key.c == GLOBALCONFIG->keybindings.upright) movePlayer(1, -1);
					else if(key.c == GLOBALCONFIG->keybindings.right) movePlayer(1, 0);
					else if(key.c == GLOBALCONFIG->keybindings.downright) movePlayer(1, 1);
					else if(key.c == GLOBALCONFIG->keybindings.down) movePlayer(0, 1);
					else if(key.c == GLOBALCONFIG->keybindings.downleft) movePlayer(-1, 1);
					else if(key.c == GLOBALCONFIG->keybindings.left) movePlayer(-1, 0);
					else if(key.c == GLOBALCONFIG->keybindings.idle) break;
				}
				else if(key.c == GLOBALCONFIG->keybindings.inventory) toggleInventory();
			default:
				break;
			}
		}
		return true;
	}

	void movePlayer(int16_t dx, int16_t dy) {
		ComponentHandle<Position> position = playerentity.component<Position>();
		position->x += dx;
		position->y += dy;
	}

	void toggleInventory() {
		if(this->render == RenderGame) this->render = RenderInventory;
		else this->render = RenderGame;
	}
};


void drawEntities(EntityManager& entities) {
	TCODColor originalcolor = TCODConsole::root->getDefaultForeground();
	entities.each<Position, Model>([](Entity, const Position &position, const Model &model) {
		TCODConsole::root->setDefaultForeground(model.color);
		TCODConsole::root->putChar(position.x, position.y, model.character);
	});
	TCODConsole::root->setDefaultForeground(originalcolor);
}

int main() {
	GameState state;

	std::shared_ptr<cpptoml::table> config;
	try {
		config = cpptoml::parse_file("7drl.toml");
	} catch(cpptoml::parse_exception &e)
	{
		std::cout << e.what() << std::endl;
		exit(1);
	}

	GLOBALCONFIG = std::make_unique<Config>(Config(config.get()));

	TCODConsole::initRoot(GLOBALCONFIG->width, GLOBALCONFIG->height, "7drl bootstrap", false);
	while (!TCODConsole::isWindowClosed()) {
		TCODConsole::root->clear();
		switch(state.render) {
			case RenderGame:
				drawEntities(state.ex.entities);
				break;
			case RenderInventory:
				std::string str = "Inventory";
				TCODConsole::root->print(GLOBALCONFIG->width/2 - str.length()/2, 1, "%s", str.c_str());
				// TODO: Iterate only player's inventory
				state.ex.entities.each<Inventory>([](Entity, const Inventory &inventory) {
					auto i = 2;
					for(auto &item : inventory.items)
					{
						TCODConsole::root->print(2, i++, "%s", item.name.c_str());
					}
				});
				break;
		}
		TCODConsole::flush();
		TCOD_key_t key;
		TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, 0, false);
		if(!state.handleInput(key))
			break;
	}
	return 0;
}
