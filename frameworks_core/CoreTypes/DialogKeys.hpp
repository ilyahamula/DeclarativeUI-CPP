#pragma once

// What a Button does for its window's keyboard (Button::isDefault() /
// isCancel()): Enter presses the DEFAULT button, Escape presses the CANCEL
// button -- whichever control has focus, except that Enter inside a multi-line
// field is a newline. One button may carry both roles.
//
// A bit set rather than an enum because of that last point. Each backend marks
// its native button with it (a Qt property, a wx client object, an ImGui
// per-frame offer) and the top-level window looks the role up when the key
// arrives, so a button destroyed with its row (ForEach) or hidden with its
// section simply stops being found -- nothing holds on to it.
//
// When several enabled, visible buttons carry a role, the first in tree order
// wins, on all three.
enum DialogKeyRole : unsigned
{
	kNoDialogKey = 0,
	kDefaultButton = 1u << 0,
	kCancelButton = 1u << 1,
};
