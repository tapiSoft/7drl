#include <iostream>

#include <libtcod.hpp>
#include <console.hpp>

#include "7drl.hpp"
#include "util/cpptoml.h"

#include <entityx/entityx.h>

using namespace entityx;

struct GameState
{
	EntityX ex;
	Entity playerentity;
	GameState() {
		playerentity = ex.entities.create();
		playerentity.assign<Position>(40, 25);
		playerentity.assign<Model>('@', TCODColor::white);
	}

	// Returns true if game should exit
	bool handleInput(TCOD_key_t key)
	{
		if(key.pressed)
		{
			switch (key.vk) {
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
			case TCODK_ESCAPE:
				return true;
			}
		}
		return false;
	}

	void movePlayer(int16_t dx, int16_t dy) {
		ComponentHandle<Position> position = playerentity.component<Position>();
		position->x += dx;
		position->y += dy;
	}
};


void drawEntities(EntityManager& entities) {
	TCODColor originalcolor = TCODConsole::root->getDefaultForeground();
	ComponentHandle<Position> position;
	ComponentHandle<Model> model;
	entities.each<Position, Model>([](Entity entity, const Position &position, const Model &model) {
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

	TCODConsole::initRoot(80, 50, "7drl bootstrap", false);
	while (!TCODConsole::isWindowClosed()) {
		TCODConsole::root->clear();
		TCODConsole::root->print(0, 0, "%s", (*config->get_as<std::string>("test")).c_str());
		drawEntities(state.ex.entities);
		TCODConsole::flush();
		TCOD_key_t key;
		TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, 0, false);
		if(state.handleInput(key)) 
			break;
	}
	return 0;
}
