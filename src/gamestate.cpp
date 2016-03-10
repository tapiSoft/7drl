#include "gamestate.hpp"
#include "util/random.hpp"
#include "7drl.hpp"

std::unique_ptr<Config> GLOBALCONFIG;

const TCODColor Level::COLOR_DARK_WALL = TCODColor(0, 0, 100);
const TCODColor Level::COLOR_LIGHT_WALL = TCODColor(120, 110, 50);
const TCODColor Level::COLOR_DARK_GROUND = TCODColor(50, 50, 150);
const TCODColor Level::COLOR_LIGHT_GROUND = TCODColor(200, 180, 50);
const TCODColor Level::COLORS[] = { Level::COLOR_DARK_GROUND, Level::COLOR_DARK_WALL, Level::COLOR_LIGHT_GROUND, Level::COLOR_LIGHT_WALL};

void monsterMovement(entityx::Entity e, GameState *state) {
	Position playerpos = *state->playerentity.component<Position>().get();
	Position newpos = *e.component<Position>().get();
	if(e.component<Behavior>()->seenPlayer) { // Attack the player! (TODO : could maybe cache the path or something)
		TCODPath path(&state->currentLevel.map);
		path.compute(newpos.x, newpos.y, playerpos.x, playerpos.y);
		int newx, newy;
		path.walk(&newx, &newy, false);
		newpos.x = newx; newpos.y = newy;
	}
	else { // Wander around aimlessly
		if(random(0, 1)) newpos.x += random(0, 1) ? 1 : -1;
		else newpos.y += random(0, 1) ? 1 : -1;
	}
	if(state->currentLevel.canMoveTo(newpos.x, newpos.y)) { // TODO : what about monster-monster collisions
		if(newpos == playerpos) {
			auto combat = e.component<Combat>();
			if(combat) {
				assert(combat->life > 0);
				auto dmg = combat->damage.getDamage();

				state->ex.events.emit<ConsoleMessage>("A monster hits you for " + std::to_string(dmg) + " damage.");
				state->ex.events.emit<Collision>(newpos.x, newpos.y);
			} else {
				state->ex.events.emit<ConsoleMessage>("A lifeless monster bumps into you.");
				state->ex.events.emit<Collision>(newpos.x, newpos.y);
			}
		} else {
			e.component<Position>()->x = newpos.x;
			e.component<Position>()->y = newpos.y;
		}
	}
}


GameState::GameState() : render(RenderGame), currentLevel(100, 100) {
	playerentity = ex.entities.create();
	playerentity.assign<Model>('@', TCODColor::white);
	playerentity.assign<Direction>(None);
	playerentity.assign<Combat>(10, Damage(1,4,1));
	playerentity.assign<Inventory>(Inventory {
		{itemList[0]},
	});

	newLevel();

	ex.systems.add<MovementSystem>(&render, playerentity, this);
	ex.systems.add<ConsoleSystem>((uint8_t)GLOBALCONFIG->consoleSize);
	ex.systems.add<DebugSystem>();
	ex.systems.configure();

	for(auto i=0; i<2; ++i)
	{
		auto monsterEmitter = ex.entities.create();
		monsterEmitter.assign<Model>('%', TCODColor::blue);
		uint16_t x, y;
		do {
			// TODO: Better randomization
			x = random(1, currentLevel.width);
			y = random(1, currentLevel.height);
		} while(!currentLevel.canMoveTo(x, y, 1));
		monsterEmitter.assign<Position>(Position {x, y});
		monsterEmitter.assign<Behavior>(Behavior([&](Entity emitter, GameState *state) {
			auto epos = emitter.component<Position>().get();
			auto emit_if_zero = random(0, 5);
			if(emit_if_zero == 0) {
				auto direction = random(1, 8);
				uint16_t destx, desty;

				// TODO: This is horrible :D
				if(direction == 1) {
					destx = epos->x-1;
					desty = epos->y-1;
				} else if(direction == 2) {
					destx = epos->x;
					desty = epos->y-1;
				} else if(direction == 3) {
					destx = epos->x+1;
					desty = epos->y-1;
				} else if(direction == 4) {
					destx = epos->x-1;
					desty = epos->y;
				} else if(direction == 5) {
					destx = epos->x+1;
					desty = epos->y;
				} else if(direction == 6) {
					destx = epos->x-1;
					desty = epos->y+1;
				} else if(direction == 7) {
					destx = epos->x;
					desty = epos->y+1;
				} else {
					destx = epos->x+1;
					desty = epos->y+1;
				}
				if(state->currentLevel.canMoveTo(destx, desty)) {
					for(auto e : ex.entities.entities_with_components<Position>())
					{
						auto pos = e.component<Position>().get();
						if(pos->x == destx && pos->y == desty) return;
					}
					createMonster(destx, desty);
				}
			}
		}));
	}
}

void GameState::createMonster() {
	uint16_t x, y;
	do {
		// TODO: Better randomization
		x = random(1, currentLevel.width);
		y = random(1, currentLevel.height);
	} while(!currentLevel.canMoveTo(x, y));
	createMonster(x, y);
}
void GameState::createMonster(uint16_t x, uint16_t y) {
	auto monster = ex.entities.create();
	monster.assign<Model>('m', TCODColor::darkerPurple);
	monster.assign<Inventory>(Inventory {{itemList[0]}});
	monster.assign<Position>(Position {x, y});
	monster.assign<Behavior>(monsterMovement);
	monster.assign<Combat>(10, Damage(1, 4, 1));
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
				auto dir = playerentity.component<Direction>().get();
				*dir = None;
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
				else {
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
					auto combat = e.component<Combat>();
					auto color = model.color;
					if(combat) {
						if(combat->life == 0) {
							color = TCODColor::darkRed;
						}
						else {
							if(e.has_component<Behavior>() && !e.component<Behavior>()->seenPlayer) {
								e.component<Behavior>()->seenPlayer = true;
								ex.events.emit<ConsoleMessage>("The monster notices you and becomes angry!");
							}
						}
					}
					TCODConsole::root->setDefaultForeground(color);
					TCODConsole::root->putChar(position.x-xoffset, position.y-yoffset, model.character);
				}
			});

			// TODO: How to draw player last without drawing it twice?
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
			TCODConsole::root->print(2, i++, "Your inventory:");
			for(auto &item : this->playerentity.component<const Inventory>()->items)
			{
				TCODConsole::root->print(2, i++, "%s", item.name.c_str());
			}
			i+=2;
			TCODConsole::root->print(2, i++, "Surroundings:");
			auto playerpos = this->playerentity.component<const Position>().get();
			ex.entities.each<const Position, const Inventory>([&](Entity e, const Position &pos, const Inventory &inv) {
				if(e != playerentity) {
					if(pos == *playerpos) {
						for(auto &item : inv.items)
						{
							TCODConsole::root->print(2, i++, "%s", item.name.c_str());
						}
					}
				}
			});
			break;
	}
	TCODConsole::flush();
}

void GameState::newLevel() {
	currentLevel.generate();
	playerentity.replace<Position>(currentLevel.initialpos);
}
