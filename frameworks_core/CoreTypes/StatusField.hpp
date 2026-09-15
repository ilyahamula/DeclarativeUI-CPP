#pragma once

#include "frameworks_core/CoreTypes/BoundValue.hpp"

#include <string>
#include <utility>
#include <vector>

// One field of a StatusBar: framework-independent data, like ToolItem and
// MenuItem. wx turns it into a wxStatusBar pane, Qt into a QLabel, ImGui draws
// the text -- all three from these two fields.
//
// Real constructors rather than aggregate initialisation because BoundValue's
// constructors are explicit: brace-initialising a member would never pick the
// binding overload, and binding is the whole point of a status bar -- the text
// is written from somewhere else and has to show up live.
struct StatusField
{
	// The framework owns this text: a literal lands here, as it cannot bind.
	StatusField(const std::string& fieldText, int fieldWidth = -1)
		: text(fieldText)
		, width(fieldWidth)
	{
	}

	// This one IS the caller's string: rewrite it from anywhere -- a handler, a
	// timer, another control's onChange -- and the field follows.
	StatusField(std::string& fieldText, int fieldWidth = -1)
		: text(fieldText)
		, width(fieldWidth)
	{
	}

	BoundValue<std::string> text;

	// Fixed pixel width, or -1 to share what the fixed fields leave over. All
	// -1 fields split the remainder equally.
	int width = -1;
};

using StatusFields = std::vector<StatusField>;

// What a STRETCH field contributes to the bar's measured width.
//
// A status bar must NOT measure its live text. Its whole job is to display a
// string written from somewhere else, so measuring the text would make an
// arriving message resize an auto-fit window -- the same trap editable fields
// avoid by measuring a content-independent floor (architecture.md's measurement
// contract). Fixed fields contribute their width and stretch fields this, and
// the engine's Expand() gives the bar the rest of the row at arrange time.
inline constexpr int kDefaultStatusFieldWidth = 120;
