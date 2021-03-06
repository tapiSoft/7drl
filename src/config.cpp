#include "config.hpp"

Config::Config(cpptoml::table *config) {
	this->keybindings = Keybindings {
		config->get_as<std::string>("upleft").value_or("y")[0],
		config->get_as<std::string>("up").value_or("k")[0],
		config->get_as<std::string>("upright").value_or("u")[0],
		config->get_as<std::string>("right").value_or("l")[0],
		config->get_as<std::string>("downright").value_or("n")[0],
		config->get_as<std::string>("down").value_or("j")[0],
		config->get_as<std::string>("downleft").value_or("b")[0],
		config->get_as<std::string>("left").value_or("h")[0],
		config->get_as<std::string>("idle").value_or("\0")[0],
		config->get_as<std::string>("inventory").value_or("\0")[0],
	};
	this->width = config->get_as<int64_t>("width").value_or(80);
	this->height = config->get_as<int64_t>("height").value_or(50);
	this->consoleSize = config->get_as<int64_t>("console_size").value_or(5);
}
