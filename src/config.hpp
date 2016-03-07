#pragma once
#include "util/cpptoml.h"

struct Keybindings {
	char upleft;
	char up;
	char upright;
	char right;
	char downright;
	char down;
	char downleft;
	char left;
	char idle;

	char inventory;
};

struct Config {
	public:
		uint16_t width;
		uint16_t height;
		Keybindings keybindings;
		Config(cpptoml::table *config);
};
