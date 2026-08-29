#pragma once

#include <utility>

#include <wx/event.h>
#include <wx/window.h>

// Mirrors an externally-owned value back into a native control.
//
// A bound ref can be written from anywhere -- another widget's handler, a worker, a
// timer -- and wx has no notification for that, so we poll on idle.
//
// The sync is driven by CHANGES TO THE REF, never by the control merely disagreeing
// with it. That distinction is load-bearing. A control can legitimately display
// something the ref does not yet know about: while a combo box's dropdown is open the
// highlighted item has moved, but the commit event has not fired, so the ref still
// holds the old value. Pushing on disagreement would reset the control mid-gesture and
// cancel the interaction outright -- the user sees a control they cannot change, and
// the commit event never arrives. Watching the ref instead means an untouched value
// produces no writes at all, whatever the control is doing.
//
// `want` reports the value the control should show and `pull` what it shows now, both
// in the control's own domain (ints for sliders, wxColour for the picker) so the
// compare never depends on float equality. `pull` is consulted only after the ref has
// actually changed, purely to skip a redundant write. `push` should use wx's
// non-notifying setter where one exists (ChangeValue over SetValue) so mirroring never
// re-enters the user's onChange.
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
	control->Bind(wxEVT_IDLE, [pull = std::move(pull), want = std::move(want), push = std::move(push),
		last = want()](wxIdleEvent& evt) mutable {
		const auto target = want();
		if (target != last)
		{
			last = target;
			if (pull() != target)
				push(target);
		}
		evt.Skip();
	});
}
