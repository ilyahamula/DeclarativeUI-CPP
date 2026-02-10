#pragma once

#include <optional>

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

struct Range
{
	int min = 0;
	int max = 100;
	std::optional<int> value = std::nullopt;
};