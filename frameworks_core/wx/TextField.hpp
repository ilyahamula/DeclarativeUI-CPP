#pragma once

#include "frameworks_core/CoreTypes/TextField.hpp"
#include "frameworks_core/wx/RefSync.hpp"

#include <wx/wx.h>

#include <memory>

// Focus and validity of a wx text field (TextField.hpp).
namespace wx_text_field
{

inline void apply(wxTextCtrl* field, TextFieldOptions options)
{
	// Invalid: a background colour, which no port counts in the best size --
	// marking a field must not move anything. wxNullColour restores the
	// native default.
	auto showInvalid = [field](bool invalid) {
		field->SetBackgroundColour(invalid ? wxColour(253, 228, 228) : wxNullColour);
		field->Refresh();
	};
	if (options.invalid.get())
		showInvalid(true);
	if (options.invalid.isBound())
	{
		auto shown = std::make_shared<bool>(options.invalid.get());
		const bool& invalid = options.invalid.get();
		bindExternalRefSync(field,
			[shown] { return *shown; },
			[&invalid] { return invalid; },
			[shown, showInvalid](bool next) {
				*shown = next;
				showInvalid(next);
			});
	}

	// Focus moves. Skipped, so the control's own focus handling still runs.
	bool* focusFlag = options.focused.isBound() ? &options.focused.get() : nullptr;
	if (options.onFocus || focusFlag != nullptr)
	{
		field->Bind(wxEVT_SET_FOCUS, [focusFlag, onFocus = options.onFocus](wxFocusEvent& event) {
			event.Skip();
			if (focusFlag != nullptr)
			{
				*focusFlag = true;
				markChanged(focusFlag);
			}
			if (onFocus)
				onFocus();
		});
	}
	if (options.onBlur || focusFlag != nullptr)
	{
		field->Bind(wxEVT_KILL_FOCUS, [focusFlag, onBlur = options.onBlur](wxFocusEvent& event) {
			event.Skip();
			if (focusFlag != nullptr)
			{
				*focusFlag = false;
				markChanged(focusFlag);
			}
			if (onBlur)
				onBlur();
		});
	}

	// Focus requests. The starting focus is set once the window is up --
	// a dialog gives focus to its first control as it is shown, which would
	// otherwise win -- and a pending call dies with the control.
	if (options.focused.get())
		field->CallAfter([field] { field->SetFocus(); });
	if (focusFlag != nullptr)
	{
		bindExternalRefSync(field,
			[field] { return wxWindow::FindFocus() == field; },
			[focusFlag] { return *focusFlag; },
			[field](bool want) {
				if (want)
					field->SetFocus();
			});
	}
}

} // namespace wx_text_field
