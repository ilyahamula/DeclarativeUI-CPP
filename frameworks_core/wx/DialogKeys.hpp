#pragma once

#include "frameworks_core/CoreTypes/DialogKeys.hpp"

#include <wx/wx.h>

// Enter / Escape for Button::isDefault() / isCancel() on wx.
//
// One wxEVT_CHAR_HOOK per top-level window (EngineSession::watch) rather than
// wx's own machinery, which covers only half of it: wxDialog's Escape handling
// finds a button by id and wxFrame has none at all, and Enter reaches a
// default button natively only when no control consumes it first -- which
// differs by port. The hook sees the key before the focused control, on every
// port and for both window kinds, so it decides the same thing everywhere:
//
//   * Escape presses the cancel button;
//   * Enter presses the default button, unless the focused control wants the
//     key -- a multi-line field (newline), or a single-line field with an
//     onEnter (wxTE_PROCESS_ENTER), whose own handler presses the default
//     button after reporting.
//
// A button carries its roles as a client object, looked up when the key
// arrives -- so a button destroyed with its row or hidden with its section is
// simply not found, and nothing holds a pointer to it.
namespace wx_dialog_keys
{

class RoleData : public wxClientData
{
public:
	explicit RoleData(unsigned roles)
		: roles(roles)
	{
	}

	unsigned roles;
};

inline void markButton(wxButton* button, unsigned roles)
{
	if (roles != kNoDialogKey)
		button->SetClientObject(new RoleData(roles));
}

// First enabled, on-screen button in `window`'s subtree carrying `role`, in
// tree order. Stops at nested top-levels: a child dialog keeps its own keys.
inline wxButton* findButton(wxWindow* window, DialogKeyRole role)
{
	for (wxWindow* child : window->GetChildren())
	{
		if (child->IsTopLevel())
			continue;
		if (auto* button = dynamic_cast<wxButton*>(child))
		{
			auto* data = dynamic_cast<RoleData*>(button->GetClientObject());
			if (data != nullptr && (data->roles & role) != 0 && button->IsEnabled() && button->IsShownOnScreen())
				return button;
		}
		if (wxButton* found = findButton(child, role))
			return found;
	}
	return nullptr;
}

// Press the button as a click would: the same wxEVT_BUTTON its handlers are
// bound to. Returns whether there was one.
inline bool press(wxWindow* window, DialogKeyRole role)
{
	wxWindow* top = wxGetTopLevelParent(window);
	wxButton* button = top != nullptr ? findButton(top, role) : nullptr;
	if (button == nullptr)
		return false;
	wxCommandEvent event(wxEVT_BUTTON, button->GetId());
	event.SetEventObject(button);
	button->ProcessWindowEvent(event);
	return true;
}

// Whether the focused control consumes Enter itself.
inline bool focusWantsEnter()
{
	auto* text = dynamic_cast<wxTextCtrl*>(wxWindow::FindFocus());
	return text != nullptr && (text->IsMultiLine() || text->HasFlag(wxTE_PROCESS_ENTER));
}

// The wxEVT_CHAR_HOOK handler body; Skip()s whatever it does not handle, so
// the window's own behaviour (a wxDialog closing on Escape) is untouched when
// there is no button for the key.
inline void onCharHook(wxWindow* top, wxKeyEvent& event)
{
	if (event.HasAnyModifiers())
	{
		event.Skip();
		return;
	}
	const int key = event.GetKeyCode();
	bool handled = false;
	if (key == WXK_ESCAPE)
		handled = press(top, kCancelButton);
	else if ((key == WXK_RETURN || key == WXK_NUMPAD_ENTER) && !focusWantsEnter())
		handled = press(top, kDefaultButton);
	if (!handled)
		event.Skip();
}

} // namespace wx_dialog_keys
