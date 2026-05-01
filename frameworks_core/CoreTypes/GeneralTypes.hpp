#pragma once

#include "frameworks_core/CoreTypes/Concepts.hpp"

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

struct Date
{
	int year  = 2000;
	int month = 1;    // 1-12
	int day   = 1;    // 1-31
};

struct Time
{
	int hour   = 0;   // 0-23
	int minute = 0;   // 0-59
	int second = 0;   // 0-59
};

struct Color
{
	float r = 0.0f;   // 0.0–1.0
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;
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