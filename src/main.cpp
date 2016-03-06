#include <iostream>

#include <libtcod.hpp>
#include <console.hpp>

#include "7drl.hpp"
#include "util/cpptoml.h"

int main() {
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
		TCODConsole::root->putChar(40, 25, PLAYER_CHAR);
		TCODConsole::flush();
	}
	return 0;
}
