#pragma once

#include <random>
#include <functional>

template<typename T>
T random(T range_start, T range_end)
{
	static std::default_random_engine gen;
	std::uniform_int_distribution<T> distribution(range_start, range_end);
	return distribution(gen);
}
