#pragma once

#include "frameworks_core/CoreTypes/BoundValue.hpp"

// Everything an Expander node and its header share.
//
// The header, like the Splitter's sash, is a framework-internal leaf rather
// than something the caller declares, so it cannot be handed its value the way
// a widget hands one to its wrapper: the expander node is built first and the
// header second, from inside it. The header therefore takes a pointer to this
// struct, which lives IN the node -- valid for the tree's lifetime on every
// backend, and the one place the engine, the header and the retained backends'
// relayout poll can all meet.
struct ExpanderState
{
	BoundValue<bool> expanded { false };

	// What the last measure pass actually laid out, latched from `expanded`
	// once the header has been measured. Arrange and the backends read THIS
	// rather than the live flag: on ImGui the header toggles during render(),
	// after the pass has already decided how big everything is, so a backend
	// reading the live flag would draw content into rectangles measure never
	// budgeted for. It is also what the wx/Qt relayout poll compares against
	// -- `applied` is what the layout shows, `expanded` what it should show --
	// which makes that poll the ordinary RefSync shape rather than a special
	// case.
	bool applied = false;
};
