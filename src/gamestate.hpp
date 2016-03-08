#pragma once

#include "7drl.hpp"
#include "items.hpp"

#include <entityx/entityx.h>

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
		Entity player;
		Level *level;
	public:
		explicit MovementSystem(Entity player, Level *level) : player(player), level(level) {}
		void update(EntityManager &em, EventManager &evm, TimeDelta) override {
			em.each<Position, Behavior>([&](Entity, Position &pos, const Behavior &b) {
				auto newpos = b.movementBehavior(pos);
				if(level->canMoveTo(newpos.x, newpos.y)) {
					if(newpos == *player.component<Position>().get()) {
						evm.emit<Collision>(newpos.x, newpos.y);
					} else pos = newpos;
				}
			});
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
				if(level->canMoveTo(destx, desty)) {
					// Check collisions with other entities
					for(auto e : em.entities_with_components<Position>())
					{
						if(e == entity) continue; // Ignore ourselves
						auto pos = e.component<Position>().get();
						if(pos->x == destx && pos->y == desty) {
							evm.emit<Collision>(pos->x, pos->y);
							return;
						}
					}

					pos.x = destx;
					pos.y = desty;
					level->refreshFov(pos);
				}
			});
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
