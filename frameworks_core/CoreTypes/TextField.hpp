#pragma once

#include "frameworks_core/CoreTypes/BoundValue.hpp"

#include <functional>

// Focus and validity of a text field (TextCtrl, PasswordInput,
// MultiLineTextCtrl) -- plain data every backend reads, like TooltipText.
//
// Text fields only, deliberately. "Focus" means the same thing on all three
// backends only for text entry: ImGui has no lasting focus for a button or a
// slider unless keyboard navigation is on, so there a field is focused exactly
// while it is being edited (its item is active), and that is what wx and Qt
// mean for a text control too.
struct TextFieldOptions
{
	// Focus moved into / out of the field.
	std::function<void()> onFocus;
	std::function<void()> onBlur;

	// Two-way when BOUND: the framework writes true/false as focus moves, and
	// the caller writing true moves focus into the field (writing false does
	// nothing -- focus has to go somewhere, and that is the user's call). A
	// snapshot `true` is the field the window opens with focus in.
	BoundValue<bool> focused { false };

	// Marked invalid: a red tint on all three backends, never a size change --
	// so marking a field cannot move anything in the window. What is invalid,
	// and when to say so (on each edit, on blur), is the caller's decision.
	BoundValue<bool> invalid { false };
};
