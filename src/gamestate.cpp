#include "gamestate.hpp"
#include "util/random.hpp"
#include "7drl.hpp"

std::unique_ptr<Config> GLOBALCONFIG;

std::vector<Item> itemList = {
	Item {
		"Potion of greater health",
		[](GameState *state) {
			state->playerentity.component<Combat>()->life += 50;
		}
	},
};

const TCODColor Level::COLOR_DARK_WALL = TCODColor(0, 0, 100);
const TCODColor Level::COLOR_LIGHT_WALL = TCODColor(120, 110, 50);
const TCODColor Level::COLOR_DARK_GROUND = TCODColor(50, 50, 150);
const TCODColor Level::COLOR_LIGHT_GROUND = TCODColor(200, 180, 50);
const TCODColor Level::COLORS[] = { Level::COLOR_DARK_GROUND, Level::COLOR_DARK_WALL, Level::COLOR_LIGHT_GROUND, Level::COLOR_LIGHT_WALL};

void moveMonsterTo(Entity e, Position newpos, GameState *state) {
	if(state->currentLevel.canMoveTo(newpos)) {
		if(!state->findEntityAt(newpos, nullptr)) {
			auto pos = e.component<Position>().get();
			state->moveEntity(*pos, newpos);
			*pos = newpos;
		}
	}
}

Position getNextStepTowards(const TCODMap& map, Position src, Position dst) {
	TCODPath path(&map);
	path.compute(src.x, src.y, dst.x, dst.y);
	int newx, newy;
	path.walk(&newx, &newy, false);
	return Position(newx, newy);
}

void monsterFriendlyMovement(entityx::Entity e, GameState *state) {
	Position playerpos = *state->playerentity.component<Position>().get();
	Position newpos = *e.component<Position>().get();
	newpos = getNextStepTowards(state->currentLevel.map, newpos, playerpos);
	if(newpos != playerpos) // TODO : attack other monsters
		moveMonsterTo(e, newpos, state);
}

void monsterAggroMovement(entityx::Entity e, GameState *state) {
	Position playerpos = *state->playerentity.component<Position>().get();
	Position newpos = *e.component<Position>().get();
	newpos = getNextStepTowards(state->currentLevel.map, newpos, playerpos);
	if(newpos == playerpos) {
		if(e.has_component<Combat>()) {
			auto combat = e.component<Combat>();
			assert(combat->life > 0);
			auto dmg = combat->damage.getDamage();

			state->ex.events.emit<ConsoleMessage>("A monster hits you for " + std::to_string(dmg) + " damage.");
			state->ex.events.emit<Collision>(newpos.x, newpos.y);

			state->playerentity.component<Combat>()->life -= dmg;
		}
		else {
			state->ex.events.emit<ConsoleMessage>("A lifeless monster bumps into you.");
			state->ex.events.emit<Collision>(newpos.x, newpos.y);
		}
	} else moveMonsterTo(e, newpos, state);
}

void monsterRandomMovement(entityx::Entity e, GameState *state) {
	Position newpos = *e.component<Position>().get();
	if(random(0, 1)) newpos.x += random(0, 1) ? 1 : -1;
	else newpos.y += random(0, 1) ? 1 : -1;
	moveMonsterTo(e, newpos, state);
}


GameState::GameState() : render(RenderGame), currentLevel(100, 100), playerStatusConsole(GLOBALCONFIG->width, 1), inventoryIndex(0) {
	playerentity = ex.entities.create();
	playerentity.assign<Model>('@', TCODColor::white);
	playerentity.assign<Direction>(0,0);
	playerentity.assign<Combat>(100, Damage(1,4,1), PLAYER);
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
		monsterEmitter.assign<Combat>(50, Damage(0, 0, 0), i == 0 ? PLAYER : HOSTILE);
		monsterEmitter.assign<Behavior>(Behavior([&](Entity emitter, GameState *state) {
			auto epos = emitter.component<Position>().get();
			auto emit_if_zero = random(0, 5);
			if(emit_if_zero == 0) {
				int8_t dx, dy;
				do {
					dx = random(-1, 1);
					dy = random(-1, 1);
				} while(dx == 0 && dy == 0);
				Position dest(epos->x + dx, epos->y + dy);
				if(state->currentLevel.canMoveTo(dest)) {
					createMonster(dest.x, dest.y, emitter.component<Combat>()->team);
				}
			}
		}));
	}
}

void GameState::createMonster(uint16_t x, uint16_t y, uint8_t team) {
	auto monster = ex.entities.create();
	monster.assign<Model>('m', TCODColor::darkerPurple);
	monster.assign<Inventory>(Inventory {{itemList[0]}});
	monster.assign<Position>(Position {x, y});
	monster.assign<Behavior>(monsterRandomMovement);
	monster.assign<Combat>(10, Damage(1, 4, 1), team);
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
		case TCODK_ENTER:
			if(this->render == RenderInventory) {
				auto &items = playerentity.component<Inventory>()->items;
				auto item = items.at(inventoryIndex);
				item.effect(this);
				items.erase(items.begin() + inventoryIndex);
			}
			break;
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
			else if(this->render == RenderInventory) {
				if(key.c == GLOBALCONFIG->keybindings.up) {
					if(inventoryIndex>0) inventoryIndex--;
				} else if(key.c == GLOBALCONFIG->keybindings.down) {
					inventoryIndex++;
				} else {
					toggleInventory();
				}
			}
		default:
			break;
		}
	}
	return true;
}

void GameState::toggleInventory() {
	if(this->render == RenderGame) {
		inventoryIndex=0;
		this->render = RenderInventory;
	}
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
						if(e.has_component<Behavior>()) {
							auto mbh = e.component<Behavior>()->movementBehavior.target<decltype(monsterRandomMovement)*>();
							if(mbh && (*mbh == monsterRandomMovement || *mbh == monsterFriendlyMovement)) {
								if(combat->team == PLAYER) e.component<Behavior>()->movementBehavior = monsterFriendlyMovement;
								else e.component<Behavior>()->movementBehavior = monsterAggroMovement;
							}
							//ex.events.emit<ConsoleMessage>("The monster notices you and becomes angry!"); TODO : doesn't work anymore
						}
					} else {
						// No combat, no life
						color = TCODColor::darkRed;
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
			TCODColor originalcolor = TCODConsole::root->getDefaultForeground();
			TCODConsole::root->print(GLOBALCONFIG->width/2 - str.length()/2, 1, "%s", str.c_str());
			auto i = 2;
			TCODConsole::root->print(2, i++, "Your inventory:");
			uint8_t ii = 0;

			uint8_t itemcount=0;
			auto playerpos = this->playerentity.component<const Position>().get();
			ex.entities.each<const Position, const Inventory>([&](Entity, const Position &pos, const Inventory &i) {
				if(pos == *playerpos) {
					itemcount += i.items.size();
				}
			});
			if(inventoryIndex>=itemcount-1) inventoryIndex=itemcount-1;
			for(auto &item : this->playerentity.component<const Inventory>()->items)
			{
				if(ii == inventoryIndex) {
					TCODConsole::root->setDefaultForeground(TCODColor::green);
				} else {
					TCODConsole::root->setDefaultForeground(originalcolor);
				}
				TCODConsole::root->print(2, i++, "%s", item.name.c_str());
				ii++;
			}
			std::cout << "inventoryIndex: " << ((int)inventoryIndex) << " " << (int)itemcount << std::endl;
			i+=2;
			TCODConsole::root->setDefaultForeground(originalcolor);
			TCODConsole::root->print(2, i++, "Surroundings:");
			ex.entities.each<const Position, const Inventory>([&](Entity e, const Position &pos, const Inventory &inv) {
				if(e != playerentity) {
					if(pos == *playerpos) {
						for(auto &item : inv.items)
						{
							if(ii == inventoryIndex) {
								TCODConsole::root->setDefaultForeground(TCODColor::green);
							} else {
								TCODConsole::root->setDefaultForeground(originalcolor);
							}
							TCODConsole::root->print(2, i++, "%s", item.name.c_str());
							ii++;
						}
					}
				}
			});
			TCODConsole::root->setDefaultForeground(originalcolor);
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
			if(entity) *entity = e;
			return true;
		}
	return false;
}
