#pragma once
#include <libtcod.hpp>
#include <vector>
#include <string>
#include "util/random.hpp"
#include <functional>
#include "config.hpp"

#include <iostream>

extern std::unique_ptr<Config> GLOBALCONFIG;

typedef std::string ConsoleMessage;

struct Position
{
	uint16_t x;
	uint16_t y;
	Position(uint16_t x, uint16_t y) : x(x), y(y) {}
	bool operator==(const Position &rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(const Position &rhs) const { return !(*this == rhs); }
};

typedef Position Collision;

enum Direction {
	Up,
	Right,
	Down,
	Left,
	UpRight,
	UpLeft,
	DownRight,
	DownLeft,
	None
};

struct Model
{
	char character;
	TCODColor color;
	Model(char character, TCODColor color) : character(character), color(color) {}
};

struct Damage {
	uint8_t numrolls;
	uint8_t dicesize;
	int8_t bonus;
	uint16_t getDamage() {
		uint16_t ret = 0;
		for (uint8_t i = 0; i < numrolls; i++)
			ret += random<int>(1, dicesize);
		if(-bonus > (int)ret) return 0;
		return ret+bonus;
	}
	Damage(uint8_t numrolls, uint8_t dicesize, int8_t bonus) : numrolls(numrolls), dicesize(dicesize), bonus(bonus) {}
};

struct Combat
{
	uint8_t life;
	Damage damage;
	Combat(uint8_t maxlife, Damage damage) : life(maxlife), damage(damage) {}
};

struct Item
{
	std::string name;
};

struct Inventory
{
	std::vector<Item> items;
};

struct Behavior
{
	std::function<Position(Position)> movementBehavior;
};

struct Cell {
	bool visible : 1;
	bool explored : 1;
	bool notwall : 1;
};

struct Level {
	TCODMap map;
	int width;
	int height;
	std::unique_ptr<Cell[]> celldata;
	Position initialpos;

	static const TCODColor COLOR_DARK_WALL,
		COLOR_LIGHT_WALL,
		COLOR_DARK_GROUND,
		COLOR_LIGHT_GROUND;
	static const TCODColor COLORS[4];

	Level(int width, int height)
		: map(width, height),
		width(width),
		height(height),
		celldata(new Cell[width*height]),
		initialpos(0, 0) {

	}

	void createRoom(int x1, int y1, int roomwidth, int roomheight) {
		std::cout << "Creating room at (" << x1 << "," << y1 << ")\n";
		for (int y = y1; y < y1+roomheight; ++y) 
			for (int x = x1; x < x1+roomwidth; ++x) {
				map.setProperties(x, y, true, true);
				celldata[y*width + x].notwall = true;
			}
	}

	void connect(int x1, int y1, int x2, int y2) {
		for(;;) {
			if (x1 != x2) {
				x1 += x1 > x2 ? -1 : 1;
			} else if (y1 != y2) {
				y1 += y1 > y2 ? -1 : 1;
			}
			else break;
			map.setProperties(x1, y1, true, true);
			celldata[y1*width + x1].notwall = true;
		}

	}

	void generate() {
		int rooms = 10;
		int minroomsize = 2;
		int maxroomsize = 7;

		Position oldcenter(0,0);

		map.clear();
		memset(celldata.get(), 0, sizeof(Cell)*width*height);
		for (int i = 0; i < rooms; ++i) {
			int roomwidth = random(minroomsize, maxroomsize);
			int roomheight = random(minroomsize, maxroomsize);
			int x1 = random(0, width - 1 - roomwidth);
			int y1 = random(0, height - 1 - roomheight);
			createRoom(x1, y1, roomwidth, roomheight);
			initialpos.x = x1 + roomwidth / 2;
			initialpos.y = y1 + roomheight / 2;
			if (i > 0) {
				connect(initialpos.x, initialpos.y, oldcenter.x, oldcenter.y);
			}
			oldcenter = initialpos;
		}

		refreshFov(initialpos);
	}

	void draw(int xoffset, int yoffset, int screenwidth, int screenheight) {
		// centering on player
		for (int y = std::max(0, -yoffset); y < screenheight; ++y) {
			int mapy = y + yoffset;
			if(mapy >= height) break;
				for (int x = std::max(0, -xoffset); x < screenwidth; ++x) {
					int mapx = x + xoffset;
					if (mapx >= width) break;
					int i = mapy*width + mapx;
					if (map.isInFov(mapx, mapy))
						celldata[i].explored = celldata[i].visible = true;
					else
						celldata[i].visible = false;

					if (celldata[i].explored)
					{
						auto c = COLORS[(celldata[i].visible ? 2 : 0) + (celldata[i].notwall ? 0 : 1)];
						TCODConsole::root->setCharBackground(x, y, c, TCOD_BKGND_SET);
					}
				}
		}
	}
	void refreshFov(Position pos) {
		map.computeFov(pos.x, pos.y, 3);
	}
	bool canMoveTo(int x, int y) {
		if(x < width && x >= 0 && y < height && y >= 0)
			return celldata[y*width + x].notwall;
		return false;
	}
};
