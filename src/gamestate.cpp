#include "gamestate.hpp"
#include "util/random.hpp"

std::unique_ptr<Config> GLOBALCONFIG;

const TCODColor Level::COLOR_DARK_WALL = TCODColor(0, 0, 100);
const TCODColor Level::COLOR_LIGHT_WALL = TCODColor(120, 110, 50);
const TCODColor Level::COLOR_DARK_GROUND = TCODColor(50, 50, 150);
const TCODColor Level::COLOR_LIGHT_GROUND = TCODColor(200, 180, 50);
const TCODColor Level::COLORS[] = { Level::COLOR_DARK_GROUND, Level::COLOR_DARK_WALL, Level::COLOR_LIGHT_GROUND, Level::COLOR_LIGHT_WALL};

GameState::GameState() : render(RenderGame), currentLevel(100, 100) {
	playerentity = ex.entities.create();
	playerentity.assign<Model>('@', TCODColor::white);
	playerentity.assign<Direction>(None);
	playerentity.assign<Inventory>(Inventory {
		{itemList[0]},
	});

	newLevel();

	ex.systems.add<ConsoleSystem>((uint8_t)GLOBALCONFIG->consoleSize);
	ex.systems.add<MovementSystem>(playerentity, &currentLevel);
	ex.systems.add<DebugSystem>();
	ex.systems.configure();

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
		monster.assign<Behavior>(Behavior {[](Position pos) { pos.x += random(-1, 1); pos.y += random(-1, 1); return pos;}});
		monster.assign<Life>(Life {4});
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
				auto dir = playerentity.component<Direction>().get();
				*dir = None;
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
			int screenwidth = TCODConsole::root->getWidth();
			int screenheight = TCODConsole::root->getHeight();
			int centerx = screenwidth / 2;
			int centery = screenheight / 2;
			int xoffset = playerentity.component<Position>()->x - centerx;
			int yoffset = playerentity.component<Position>()->y - centery;

			currentLevel.draw(xoffset, yoffset, screenwidth, screenheight);
			TCODColor originalcolor = TCODConsole::root->getDefaultForeground();
			ex.entities.each<const Position, const Model>([&](Entity e, const Position &position, const Model &model) {
				if(currentLevel.map.isInFov(position.x, position.y)) {
					auto life = e.component<Life>();
					auto color = model.color;
					if(life) {
						if(life->amount == 0) {
							color = TCODColor::darkRed;
						}
					}
					TCODConsole::root->setDefaultForeground(color);
					TCODConsole::root->putChar(position.x-xoffset, position.y-yoffset, model.character);
				}
			});

			TCODConsole::root->setDefaultForeground(originalcolor);
			auto playerpos = playerentity.component<Position>().get();
			auto playermodel = playerentity.component<Model>().get();
			TCODConsole::root->putChar(playerpos->x-xoffset, playerpos->y-yoffset, playermodel->character);

			TCODConsole::root->setDefaultForeground(originalcolor);
			auto &messageconsole = ex.systems.system<ConsoleSystem>()->GetConsole();
			TCODConsole::blit(&messageconsole, 0, 0, GLOBALCONFIG->width, GLOBALCONFIG->consoleSize, TCODConsole::root, 0, GLOBALCONFIG->height-GLOBALCONFIG->consoleSize);
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
