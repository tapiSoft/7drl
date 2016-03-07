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
		config->get_as<std::string>("idle").value_or("\0")[0]
	};
}
