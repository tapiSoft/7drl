#include <iostream>

#include <libtcod.hpp>
#include <console.hpp>

#include "7drl.hpp"
#include "items.hpp"
#include "config.hpp"
#include "gamestate.hpp"
#include "util/cpptoml.h"

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
		state.renderState();
		TCOD_key_t key;
		TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, 0, false);
		if(state.handleInput(key))
		{
			state.ex.systems.update_all(0);
		}
		else break;
	}
	return 0;
}
