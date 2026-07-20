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

    bool operator==(const Size&) const = default;
};

// ── Layout engine types (docs/specs/custom_layout_system/architecture.md) ──

// Absolute rectangle assigned to a node by the arrange pass.
struct Rect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    bool operator==(const Rect&) const = default;
};

// Space offered to a node during the measure pass (constraints flow down,
// desired sizes return up). Enables width-dependent height (text wrapping).
struct Constraints
{
    int maxWidth = 0;
    int maxHeight = 0;

    bool operator==(const Constraints&) const = default;
};

// Cross-axis participation of a node in its parent's box.
enum class Align
{
    Start,
    Center,
    End,
    Stretch
};

// Per-side border accumulated from LayoutFlags::Border() calls.
struct EdgeInsets
{
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    bool operator==(const EdgeInsets&) const = default;
};

// Kind of a layout node; drives the kind-dependent defaults
// (Leaf -> Align::Start, containers -> Align::Stretch).
enum class NodeKind
{
    Box,
    GroupBox,
    TabPanel,
    Leaf
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

enum class MessageBoxStyle { Info, Warning, Error, Question };

enum class MessageBoxButtons { OK, OKCancel, YesNo, YesNoCancel };

enum class MessageBoxResult { OK, Cancel, Yes, No };