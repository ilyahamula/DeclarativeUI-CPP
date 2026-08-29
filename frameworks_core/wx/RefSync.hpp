#pragma once

#include <utility>

#include <wx/event.h>
#include <wx/window.h>

// Mirrors an externally-owned value back into a native control.
//
// A bound ref can be written from anywhere -- another widget's handler, a worker, a
// timer -- and wx has no notification for that, so we poll on idle. `pull` reports what
// the control currently shows and `want` what it should show; both work in the control's
// own domain (ints for sliders, wxColour for the picker) so the compare never depends on
// float equality. Comparing against the control instead of a cached copy makes user
// edits self-cancelling: the control's own event handler has already written the ref, so
// the two agree and nothing is pushed back -- no redundant repaint, no disturbed caret
// or selection. `push` should use wx's non-notifying setter where one exists
// (ChangeValue over SetValue) so mirroring never re-enters the user's onChange.
//
// Capture the bound value by reference, never the wrapper: the ref belongs to the caller
// and outlives everything here, whereas wrapper and window teardown order is not fixed.
//
// Idle fires whenever the event queue drains. We deliberately do not RequestMore(),
// which would keep the loop awake and spin the CPU; a value written while the UI is
// completely quiet lands on the next event instead.
template <typename Pull, typename Want, typename Push>
void bindExternalRefSync(wxWindow* control, Pull pull, Want want, Push push)
{
	control->Bind(wxEVT_IDLE, [pull = std::move(pull), want = std::move(want), push = std::move(push)](wxIdleEvent& evt) {
		const auto target = want();
		if (pull() != target)
			push(target);
		evt.Skip();
	});
}
