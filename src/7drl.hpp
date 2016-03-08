#pragma once
#include <libtcod.hpp>
#include <vector>
#include <string>
#include "config.hpp"

extern std::unique_ptr<Config> GLOBALCONFIG;

struct Position
{
	uint16_t x;
	uint16_t y;
	Position(uint16_t x, uint16_t y) : x(x), y(y) {}
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

struct Item
{
	std::string name;
};

struct Inventory
{
	std::vector<Item> items;
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

	void generate() {
		map.clear();
		memset(celldata.get(), 0, sizeof(Cell)*width*height);
		int x1 = rand() % (width - 10), y1 = rand() % (height - 10);
		int x2 = x1 + 5 + (rand() % 5), y2 = y1 + 5 + (rand() % 5);
		for (int i = x1; i < x2; ++i)
			for (int j = y1; j < y2; ++j) {
				map.setProperties(i, j, true, true);
				celldata[i*width + j].notwall = true;
			}
		initialpos.x = (x1 + x2) / 2;
		initialpos.y = (y1 + y2) / 2;
		refreshFov(initialpos);
	}

	void draw() {
		for (int x = 0; x < width; ++x)
			for (int y = 0; y < height; ++y) {
				int i = x*width + y;
				if (map.isInFov(x, y))
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
	void refreshFov(Position pos) { map.computeFov(pos.x, pos.y, 3); }
	bool canMoveTo(int x, int y) {
		if(x < width && x >= 0 && y < height && y >= 0)
			return celldata[x*width + y].notwall;
		return false;
	}
};
