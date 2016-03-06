#include <iostream>

#include <libtcod.hpp>
#include <console.hpp>

#include "7drl.hpp"
#include "util/cpptoml.h"

#include <entityx/entityx.h>

using namespace entityx;

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
	EntityX ex;

	auto playerentity=ex.entities.create();
	playerentity.assign<Position>(40, 25);
	playerentity.assign<Model>('@', TCODColor::white);

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
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, nullptr, nullptr);
		TCODConsole::root->clear();
		TCODConsole::root->print(0, 0, "%s", (*config->get_as<std::string>("test")).c_str());
		drawEntities(ex.entities);
		TCODConsole::flush();
	}
	return 0;
}
