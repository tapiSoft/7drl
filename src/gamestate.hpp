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

	// Returns false if game should exit
	bool handleInput(TCOD_key_t key);
	void toggleInventory();
	void renderState();
	void newLevel();
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
				em.each<Position, Direction>([&](Entity entity, Position &pos, Direction &d) {
					int destx = pos.x;
					int desty = pos.y;
					auto amount = 1;
					switch(d) {
						case UpRight:
							destx += amount;
							desty -= amount;
							break;
						case UpLeft:
							destx -= amount;
							desty -= amount;
							break;
						case Up:
							desty -= amount;
							break;
						case Right:
							destx += amount;
							break;
						case DownRight:
							destx += amount;
							desty += amount;
							break;
						case Down:
							desty += amount;
							break;
						case DownLeft:
							destx -= amount;
							desty += amount;
							break;
						case Left:
							destx -= amount;
							break;
						case None:
							break;
					}
					if(state->currentLevel.canMoveTo(destx, desty)) {
						// Check collisions with other entities
						for(auto e : em.entities_with_components<Position>())
						{
							if(e == entity) continue; // Ignore ourselves
							auto pos = e.component<Position>().get();
							auto combat = e.component<Combat>();
							if(pos->x == destx && pos->y == desty) {
								if(combat)  {
									if(combat->life > 0) {
										evm.emit<Collision>(pos->x, pos->y);
										auto dmg = entity.component<Combat>()->damage.getDamage();
										evm.emit<ConsoleMessage>("You hit the monster in the nose for " + std::to_string(dmg) + " damage.");
										if(combat->life <= dmg) {
											combat->life = 0;
											evm.emit<ConsoleMessage>("The monster dies...");
											e.component<Behavior>().remove();
										}
										else combat->life -= dmg;
										return;
									} else {
										evm.emit<ConsoleMessage>("There's a dead monster here.");
									}
								} else {
									evm.emit<Collision>(pos->x, pos->y);
									evm.emit<ConsoleMessage>("You bump into a lifeless thing.");
									return;
								}
							}
						}

						pos.x = destx;
						pos.y = desty;
						state->currentLevel.refreshFov(pos);
					}
					em.each<Behavior>([&](Entity e, const Behavior &b) {
						b.movementBehavior(e, state);
				});
				});
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
