#pragma once
#include <libtcod.hpp>
#include <vector>
#include <string>

const char PLAYER_CHAR = '@';

struct Position
{
	uint16_t x;
	uint16_t y;
	Position(uint16_t x, uint16_t y) : x(x), y(y) {}
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
