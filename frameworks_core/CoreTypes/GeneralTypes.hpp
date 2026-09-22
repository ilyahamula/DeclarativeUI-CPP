#pragma once

#include "frameworks_core/CoreTypes/ValueConcepts.hpp"

#include <algorithm>
#include <cmath>
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
    ScrollPanel,
    Splitter,
    Expander,
    Leaf
};

// Which axes a ScrollPanel scrolls. An axis that scrolls is measured against
// the viewport cap rather than its content; an axis that does not behaves like
// a Box's.
enum class ScrollAxis
{
    Vertical,
    Horizontal,
    Both
};

inline bool scrollsHorizontally(ScrollAxis axis)
{
    return axis == ScrollAxis::Horizontal || axis == ScrollAxis::Both;
}

inline bool scrollsVertically(ScrollAxis axis)
{
    return axis == ScrollAxis::Vertical || axis == ScrollAxis::Both;
}

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

// What happens to a picture's PIXELS inside the rectangle the engine gave it.
// Deliberately not a LayoutFlag: the flags describe how a node takes part in
// its parent's box, and every mode here leaves the node's frame exactly as it
// was -- only the paint inside changes.
enum class ScaleMode
{
	Stretch,   // fill the frame exactly, aspect ratio ignored -- the default,
	           // and what the framework did before there was a choice
	Fit,       // largest uniform scale that fits INSIDE the frame; letterboxed
	Fill,      // smallest uniform scale that COVERS the frame; cropped
	Center     // no scaling at all; centred, and cropped where it overflows
};

// Where the picture lands inside the frame, in FRAME-LOCAL coordinates.
//
// The result may deliberately fall outside the frame -- Fill and Center crop --
// so every backend clips against the frame rather than trusting this rectangle
// to be inside it. It lives here, framework-independent, because R13.3 asks for
// the same pixels on all three backends and three copies of this arithmetic
// would not stay the same for long.
inline Rect scaledImageRect(ScaleMode mode, const Size& image, const Size& frame)
{
	const int frameW = std::max(frame.width, 0);
	const int frameH = std::max(frame.height, 0);
	if (image.width <= 0 || image.height <= 0)
		return Rect { 0, 0, frameW, frameH };

	Size target { frameW, frameH };
	if (mode == ScaleMode::Center)
	{
		target = image;
	}
	else if (mode != ScaleMode::Stretch)
	{
		// Fit takes the smaller ratio, so the picture ends up inside the frame;
		// Fill the larger, so the frame ends up inside the picture.
		const double byWidth  = (double)frameW / (double)image.width;
		const double byHeight = (double)frameH / (double)image.height;
		const double scale = (mode == ScaleMode::Fit)
			? std::min(byWidth, byHeight)
			: std::max(byWidth, byHeight);
		target.width  = std::max(1, (int)std::lround(image.width  * scale));
		target.height = std::max(1, (int)std::lround(image.height * scale));
	}

	// Anything that is not the frame's own size is centred in it, which is what
	// makes a letterbox symmetrical and a crop take the middle of the picture.
	return Rect {
		(frameW - target.width)  / 2,
		(frameH - target.height) / 2,
		target.width,
		target.height
	};
}

// Where a label's text sits in the frame the engine gave it. Only visible once
// the frame is WIDER than the text: a leaf sits at its desired size by default,
// so this pairs with Expand(), a SizeGroup or a Grid band.
enum class TextAlign
{
	Left,
	Center,
	Right
};

enum class MessageBoxStyle { Info, Warning, Error, Question };

enum class MessageBoxButtons { OK, OKCancel, YesNo, YesNoCancel };

enum class MessageBoxResult { OK, Cancel, Yes, No };

// What a file dialog is for. Drives which native dialog the retained backends
// open and which of its three shapes the ImGui browser draws.
enum class FileMode
{
	Open,       // pick an existing file
	Save,       // name a file, existing or not (overwrite prompt is the platform's)
	Directory   // pick a folder
};
