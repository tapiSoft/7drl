#include "gamestate.hpp"
#include "util/random.hpp"
#include "7drl.hpp"

std::unique_ptr<Config> GLOBALCONFIG;

const TCODColor Level::COLOR_DARK_WALL = TCODColor(0, 0, 100);
const TCODColor Level::COLOR_LIGHT_WALL = TCODColor(120, 110, 50);
const TCODColor Level::COLOR_DARK_GROUND = TCODColor(50, 50, 150);
const TCODColor Level::COLOR_LIGHT_GROUND = TCODColor(200, 180, 50);
const TCODColor Level::COLORS[] = { Level::COLOR_DARK_GROUND, Level::COLOR_DARK_WALL, Level::COLOR_LIGHT_GROUND, Level::COLOR_LIGHT_WALL};

void moveMonsterTo(Entity e, Position newpos, GameState *state) {
	if(state->currentLevel.canMoveTo(newpos)) {
		auto pos = e.component<Position>().get();
		state->moveEntity(*pos, newpos);
		*pos = newpos;
	}
}

void monsterAggroMovement(entityx::Entity e, GameState *state) {
	Position playerpos = *state->playerentity.component<Position>().get();
	Position newpos = *e.component<Position>().get();
	TCODPath path(&state->currentLevel.map);
	path.compute(newpos.x, newpos.y, playerpos.x, playerpos.y);
	int newx, newy;
	path.walk(&newx, &newy, false);
	newpos.x = newx; newpos.y = newy;
	if(newpos == playerpos) {
		if(e.has_component<Combat>()) {
			auto combat = e.component<Combat>();
			assert(combat->life > 0);
			auto dmg = combat->damage.getDamage();

			state->ex.events.emit<ConsoleMessage>("A monster hits you for " + std::to_string(dmg) + " damage.");
			state->ex.events.emit<Collision>(newpos.x, newpos.y);
		}
		else {
			state->ex.events.emit<ConsoleMessage>("A lifeless monster bumps into you.");
			state->ex.events.emit<Collision>(newpos.x, newpos.y);
		}
	} else moveMonsterTo(e, newpos, state);
}

void monsterRandomMovement(entityx::Entity e, GameState *state) {
	Position playerpos = *state->playerentity.component<Position>().get();
	Position newpos = *e.component<Position>().get();
	if(random(0, 1)) newpos.x += random(0, 1) ? 1 : -1;
	else newpos.y += random(0, 1) ? 1 : -1;
	moveMonsterTo(e, newpos, state);
}


GameState::GameState() : render(RenderGame), currentLevel(100, 100), playerStatusConsole(GLOBALCONFIG->width, 1) {
	playerentity = ex.entities.create();
	playerentity.assign<Model>('@', TCODColor::white);
	playerentity.assign<Direction>(0,0);
	playerentity.assign<Combat>(100, Damage(1,4,1));
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
		} while(!currentLevel.canMoveTo(Position(x, y), 1));
		monsterEmitter.assign<Position>(Position {x, y});
		monsterEmitter.assign<Behavior>(Behavior([&](Entity emitter, GameState *state) {
			auto epos = emitter.component<Position>().get();
			auto emit_if_zero = random(0, 5);
			if(emit_if_zero == 0) {
				auto direction = random(1, 8);
				int8_t dx, dy;
				do {
					dx = random(-1, 1);
					dy = random(-1, 1);
				} while(dx == 0 && dy == 0);
				Position dest(epos->x + dx, epos->y + dy);
				if(state->currentLevel.canMoveTo(dest)) {
					createMonster(dest.x, dest.y, emitter.component<Behavior>()->friendlyToPlayer);
				}
			}
		}, false));
	}
}

void GameState::createMonster(uint16_t x, uint16_t y, bool friendly) {
	auto monster = ex.entities.create();
	monster.assign<Model>('m', TCODColor::darkerPurple);
	monster.assign<Inventory>(Inventory {{itemList[0]}});
	monster.assign<Position>(Position {x, y});
	monster.assign<Behavior>(monsterRandomMovement, friendly);
	monster.assign<Combat>(10, Damage(1, 4, 1));
}

// Returns false if game should exit
bool GameState::handleInput(TCOD_key_t key) {
	if(key.pressed)
	{
		auto dir = playerentity.component<Direction>().get();
		*dir = Direction(0, 0);
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
				if(key.c == GLOBALCONFIG->keybindings.upleft || key.c == GLOBALCONFIG->keybindings.downleft || key.c == GLOBALCONFIG->keybindings.left)
					dir->dx = -1;
				else if(key.c == GLOBALCONFIG->keybindings.upright || key.c == GLOBALCONFIG->keybindings.downright || key.c == GLOBALCONFIG->keybindings.right)
					dir->dx = 1;

				if(key.c == GLOBALCONFIG->keybindings.upleft || key.c == GLOBALCONFIG->keybindings.up || key.c == GLOBALCONFIG->keybindings.upright)
					dir->dy = -1;
				else if(key.c == GLOBALCONFIG->keybindings.downright || key.c == GLOBALCONFIG->keybindings.downleft || key.c == GLOBALCONFIG->keybindings.down)
					dir->dy = 1;
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
							if(e.has_component<Behavior>()) {
								e.component<Behavior>()->movementBehavior = monsterAggroMovement;
								//ex.events.emit<ConsoleMessage>("The monster notices you and becomes angry!"); TODO : doesn't work anymore
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

			playerStatusConsole.setDefaultForeground(originalcolor);
			playerStatusConsole.clear();
			playerStatusConsole.print(0, 0, "Health: %d", playerentity.component<Combat>().get()->life);
			TCODConsole::blit(&playerStatusConsole, 0, 0, GLOBALCONFIG->width, 1, TCODConsole::root, 0, GLOBALCONFIG->height-GLOBALCONFIG->consoleSize-1);

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

bool GameState::findEntityAt(Position p, Entity* entity) {
	ComponentHandle<Position> position;
	ComponentHandle<Combat> combat;
	for (Entity e : ex.entities.entities_with_components(position, combat))
		if(*position.get() == p) {
			*entity = e;
			return true;
		}
	return false;
}
