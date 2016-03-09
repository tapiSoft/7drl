#include <iostream>

#include <libtcod.hpp>
#include <console.hpp>

#include "7drl.hpp"
#include "items.hpp"
#include "config.hpp"
#include "gamestate.hpp"
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
	GLOBALCONFIG = std::make_unique<Config>(Config(config.get()));

	TCODConsole::setCustomFont("terminal.png", TCOD_FONT_LAYOUT_TCOD | TCOD_FONT_TYPE_GREYSCALE, 0, 0);
	TCODConsole::initRoot(GLOBALCONFIG->width, GLOBALCONFIG->height, "7drl bootstrap", false);
	GameState state;
	while (!TCODConsole::isWindowClosed()) {
		state.renderState();
		TCOD_key_t key;
		TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, 0, false);
		if(state.handleInput(key))
		{
			state.ex.systems.update<MovementSystem>(0);
			state.ex.systems.update<ConsoleSystem>(0);
			state.ex.systems.update<DebugSystem>(0);
		}
		else break;
	}
	return 0;
}
