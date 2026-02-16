#pragma once

#include "Concepts.hpp"

enum class Orientation
{
    Horizontal,
    Vertical
};

struct Position
{
    int x;
    int y;
};

struct Size
{
    int width;
    int height;
};

template <SliderValue T>
struct Range
{
	T min{};
	T max = T(100);
	T step = T(1);
};

enum class Side : int
{
	Left   = 1 << 0,
	Right  = 1 << 1,
	Top    = 1 << 2,
	Bottom = 1 << 3,
	All    = Left | Right | Top | Bottom
};

constexpr Side operator|(Side lhs, Side rhs)
{
	return static_cast<Side>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

constexpr bool operator&(Side lhs, Side rhs)
{
	return (static_cast<int>(lhs) & static_cast<int>(rhs)) != 0;
}