#include "gamestate.hpp"

std::unique_ptr<Config> GLOBALCONFIG;

const TCODColor Level::COLOR_DARK_WALL = TCODColor(0, 0, 100);
const TCODColor Level::COLOR_LIGHT_WALL = TCODColor(120, 110, 50);
const TCODColor Level::COLOR_DARK_GROUND = TCODColor(50, 50, 150);
const TCODColor Level::COLOR_LIGHT_GROUND = TCODColor(200, 180, 50);
const TCODColor Level::COLORS[] = { Level::COLOR_DARK_GROUND, Level::COLOR_DARK_WALL, Level::COLOR_LIGHT_GROUND, Level::COLOR_LIGHT_WALL};

GameState::GameState() : render(RenderGame), currentLevel(20, 20) {
	playerentity = ex.entities.create();
	playerentity.assign<Model>('@', TCODColor::white);
	playerentity.assign<Inventory>(Inventory {
		{itemList[0]},
	});
	newLevel();
}

// Returns false if game should exit
bool GameState::handleInput(TCOD_key_t key) {
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

void GameState::movePlayer(int16_t dx, int16_t dy) {
	ComponentHandle<Position> position = playerentity.component<Position>();
	if (currentLevel.canMoveTo(position->x + dx, position->y + dy)) {
		position->x += dx;
		position->y += dy;
		currentLevel.refreshFov(*position.get());
	}
}

void GameState::toggleInventory() {
	if(this->render == RenderGame) this->render = RenderInventory;
	else this->render = RenderGame;
}

void GameState::renderState() {
	TCODConsole::root->clear();
	switch(this->render) {
		case RenderGame: {
			currentLevel.draw();
			TCODColor originalcolor = TCODConsole::root->getDefaultForeground();
			this->ex.entities.each<const Position, const Model>([](const Entity&, const Position &position, const Model &model) {
				TCODConsole::root->setDefaultForeground(model.color);
				TCODConsole::root->putChar(position.x, position.y, model.character);
			});
			TCODConsole::root->setDefaultForeground(originalcolor);
			break;
		}
		case RenderInventory:
			std::string str = "Inventory";
			TCODConsole::root->print(GLOBALCONFIG->width/2 - str.length()/2, 1, "%s", str.c_str());
			auto i = 2;
			for(auto &item : this->playerentity.component<const Inventory>()->items)
			{
				TCODConsole::root->print(2, i++, "%s", item.name.c_str());
			}
			break;
	}
	TCODConsole::flush();
}

void GameState::newLevel() {
	currentLevel.generate();
	playerentity.replace<Position>(currentLevel.initialpos);
}
