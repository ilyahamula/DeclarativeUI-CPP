#pragma once

#include "frameworks_core/CoreTypes/Concepts.hpp"

#include <string>
#include <vector>

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
    Grid,
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

// One node of a TreeView's item tree. Framework-independent by design: every
// backend walks this same structure to build (wx/Qt) or draw (ImGui) its own
// items, so a tree is described once and looks the same everywhere.
//
// `expanded` is the INITIAL state only. Once the control exists the user owns
// it -- wx/Qt keep it in the native control and ImGui in its own ID-keyed
// storage, because the ImGui tree is rebuilt every frame and could not
// remember it otherwise.
struct TreeItem
{
	std::string label;
	std::vector<TreeItem> children;
	bool expanded = false;
};

// One column of a Table. Framework-independent by design, exactly as TreeItem
// is: every backend walks this same list to build (wx/Qt) or draw (ImGui) its
// own header, so a table is described once and reads the same everywhere.
//
// `width` of -1 sizes the column from its widest cell, matching withSize()'s
// "-1 means measure me" convention everywhere else in the framework.
//
// `sortable` and `editable` are per column rather than per table because
// neither usually applies to all of them: an id column sorts where a free-text
// note does not, and a computed column is shown but never typed into.
struct TableColumn
{
	std::string label;
	int width = -1;
	bool sortable = false;
	bool editable = false;
};

// A table's cells, as text. A row's index in this vector is its ORIGINAL
// position -- what a Table's int binding reports, and the only row identity
// that stays meaningful once the user sorts a column.
using TableRow = std::vector<std::string>;
using TableRows = std::vector<TableRow>;

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