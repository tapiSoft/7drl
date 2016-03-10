#pragma once

#include "7drl.hpp"
#include "items.hpp"

#include <entityx/entityx.h>
#include <deque>

using namespace entityx;

enum RenderState {
	RenderGame,
	RenderInventory,
};

struct GameState
{
	RenderState render;
	EntityX ex;
	Entity playerentity;
	std::vector<Entity> monsters;
	Level currentLevel;
	GameState();

	TCODConsole playerStatusConsole;

	// Returns false if game should exit
	bool handleInput(TCOD_key_t key);
	void toggleInventory();
	void renderState();
	void newLevel();
	void createMonster();
	void createMonster(uint16_t, uint16_t);
	void moveEntity(Position oldpos, Position newpos) {
		currentLevel.setEntityPresent(oldpos, false);
		currentLevel.setEntityPresent(newpos, true);
	}
	bool findEntityAt(Position, Entity*);
};

class MovementSystem : public System<MovementSystem> {
	private:
		RenderState *renderState;
		Entity player;
		GameState *state;
	public:
		explicit MovementSystem(RenderState *renderState, Entity player, GameState *state) : renderState(renderState), player(player), state(state) {}
		void update(EntityManager &em, EventManager &evm, TimeDelta) override {
			if(*renderState == RenderGame) {
				Direction d = *player.component<Direction>().get();
				if(d.dx || d.dy) {
					Position newpos = *player.component<Position>().get();
					newpos.x += d.dx;
					newpos.y += d.dy;
					if(state->currentLevel.canMoveTo(newpos)) {
						state->moveEntity(*player.component<Position>().get(), newpos);
						*player.component<Position>().get() = newpos;
						if(state->currentLevel.itemPresent(newpos))
							evm.emit<ConsoleMessage>("There's a dead monster here.");
					}
					else if(state->currentLevel.entityPresent(newpos)) { // combat
						Entity e;
						if(state->findEntityAt(newpos, &e)) {
							auto combat = e.component<Combat>();
							evm.emit<Collision>(newpos.x, newpos.y);
							auto dmg = player.component<Combat>()->damage.getDamage();
							evm.emit<ConsoleMessage>("You hit the monster in the nose for " + std::to_string(dmg) + " damage.");
							if(combat->life <= dmg) {
								combat->life = 0;
								evm.emit<ConsoleMessage>("The monster dies...");
								e.component<Behavior>().remove();
								e.component<Combat>().remove();
								state->currentLevel.setItemPresent(newpos, true);
								state->currentLevel.setEntityPresent(newpos, false);
							}
							else combat->life -= dmg;
						}
						else {
							// WutFace
						}
					}
				}

				// Should wake up monsters here
				em.each<Behavior>([&](Entity e, const Behavior &b) {
					b.movementBehavior(e, state);
				});
				state->currentLevel.refreshFov(*player.component<Position>().get());
			}
		}
};

struct DebugSystem : public System<DebugSystem>, public Receiver<DebugSystem> {
	void configure(EventManager &evm) override {
		evm.subscribe<Collision>(*this);
	}
	void update(EntityManager &, EventManager &, TimeDelta) override {}
	void receive(const Collision &c) {
		std::cout << "[DebugSystem]: Player ran into a monster at (" << c.x << ", " << c.y << ")" << std::endl;
	}
};

class ConsoleSystem : public System<ConsoleSystem>, public Receiver<ConsoleSystem> {
	private:
		uint8_t bufferSize;
		TCODConsole messageConsole;
		std::deque<std::string> messageBuffer;
	public:
		explicit ConsoleSystem(uint8_t buffersize) : bufferSize(buffersize), messageConsole(GLOBALCONFIG->width, buffersize) {}
		void configure(EventManager &evm) override {
			evm.subscribe<ConsoleMessage>(*this);
			messageConsole.setDefaultBackground(TCODColor::darkestGrey);
			messageConsole.setDefaultForeground(TCODColor::white);
			messageConsole.clear();
		}
		void update(EntityManager &, EventManager &, TimeDelta) override {
			messageConsole.clear();
			for(size_t i=0; i<messageBuffer.size(); ++i)
			{
				messageConsole.print(1, i+1, "%s", messageBuffer[i].c_str());
			}
		}
		void receive(const ConsoleMessage &msg) {
			messageBuffer.push_back(msg);
			if(messageBuffer.size() >= bufferSize) {
				messageBuffer.pop_front();
			}
		}
		TCODConsole &GetConsole() {
			return messageConsole;
		}
};
