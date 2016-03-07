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
};

struct Config {
	public:
		Keybindings keybindings;
		Config(cpptoml::table *config);
};
