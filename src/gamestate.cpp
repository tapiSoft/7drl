#include "gamestate.hpp"
#include "util/random.hpp"

std::unique_ptr<Config> GLOBALCONFIG;

const TCODColor Level::COLOR_DARK_WALL = TCODColor(0, 0, 100);
const TCODColor Level::COLOR_LIGHT_WALL = TCODColor(120, 110, 50);
const TCODColor Level::COLOR_DARK_GROUND = TCODColor(50, 50, 150);
const TCODColor Level::COLOR_LIGHT_GROUND = TCODColor(200, 180, 50);
const TCODColor Level::COLORS[] = { Level::COLOR_DARK_GROUND, Level::COLOR_DARK_WALL, Level::COLOR_LIGHT_GROUND, Level::COLOR_LIGHT_WALL};

GameState::GameState() : render(RenderGame), currentLevel(20, 20) {

	playerentity = ex.entities.create();
	playerentity.assign<Model>('@', TCODColor::white);
	playerentity.assign<Direction>(None);
	playerentity.assign<Inventory>(Inventory {
		{itemList[0]},
	});

	newLevel();
	ex.systems.add<MovementSystem>(&currentLevel);

	for(auto i=0; i<4; ++i)
	{
		auto monster = ex.entities.create();
		monster.assign<Model>('m', TCODColor::darkerPurple);
		monster.assign<Inventory>(Inventory {{itemList[0]}});
		uint16_t x, y;
		do {
			// TODO: Better randomization
			x = random(1, currentLevel.width);
			y = random(1, currentLevel.height);
		} while(!currentLevel.canMoveTo(x, y));
		monster.assign<Position>(Position {x, y});
	}
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
				auto dir = playerentity.component<Direction>().get();
				if(key.c == GLOBALCONFIG->keybindings.upleft) *dir = UpLeft;
				else if(key.c == GLOBALCONFIG->keybindings.up) *dir = Up;
				else if(key.c == GLOBALCONFIG->keybindings.upright) *dir = UpRight;
				else if(key.c == GLOBALCONFIG->keybindings.right) *dir = Right;
				else if(key.c == GLOBALCONFIG->keybindings.downright) *dir = DownRight;
				else if(key.c == GLOBALCONFIG->keybindings.down) *dir = Down;
				else if(key.c == GLOBALCONFIG->keybindings.downleft) *dir = DownLeft;
				else if(key.c == GLOBALCONFIG->keybindings.left) *dir = Left;
				else if(key.c == GLOBALCONFIG->keybindings.idle) {
					*dir = None;
					break;
				}
			}
			else if(key.c == GLOBALCONFIG->keybindings.inventory) toggleInventory();
		default:
			break;
		}
	}
	return true;
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
			ex.entities.each<const Position, const Model>([](const Entity&, const Position &position, const Model &model) {
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
