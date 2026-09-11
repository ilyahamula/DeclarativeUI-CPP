#pragma once

#include "frameworks_core/CoreTypes/BoundValue.hpp"
#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

// Everything a Splitter node and its sash share.
//
// The sash is a framework-internal leaf rather than something the caller
// declares, so it cannot be handed its value the way a widget hands one to its
// wrapper: the splitter node is built first and the sash second, from inside
// it. The sash therefore takes a pointer to this struct, which lives IN the
// node -- valid for the tree's lifetime on every backend, and the one place the
// engine, the sash and the retained backends' relayout poll can all meet.
//
// `position` is the FIRST pane's main-axis extent, not an absolute coordinate,
// and -1 means "half", resolved at arrange time once the area is known.
struct SplitterState
{
	// Floor a pane may not be dragged below. Small enough to stay out of the
	// way, large enough that a pane can never be dragged out of existence.
	static constexpr int kDefaultMinPane = 40;

	// The sash's own thickness. Framework-owned rather than native: no backend
	// contributes a splitter we drive (a wxSplitterWindow or QSplitter would
	// own its children's geometry and fight the engine), so there is no native
	// metric to read and one number keeps the three frames identical.
	static constexpr int kSashThickness = 6;

	BoundValue<int> position { -1 };
	int minFirst = kDefaultMinPane;
	int minSecond = kDefaultMinPane;
	Orientation orientation = Orientation::Horizontal;

	// Filled by the arrange pass; read by the sash while dragging and by the
	// wx/Qt relayout poll. `resolved` is what the layout actually shows --
	// `position` clamped into [lowerBound, upperBound] -- so a drag past a
	// floor cannot bank up an out-of-range value the user then has to drag
	// all the way back through before the sash moves again.
	int resolved = 0;
	int lowerBound = 0;
	int upperBound = 0;
};
