#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <wx/frame.h>
#include <wx/timer.h>

#include <string>
#include <vector>

// The native half of ToastWrapper on wx: a borderless, always-on-top frame
// shown WITHOUT activation, placed at the bottom-right of the active window.
//
// Deliberately not wxNotificationMessage: that goes to the OS notification
// centre, outside the application and (on macOS) nowhere at all for an
// unbundled executable, while Qt and ImGui draw an in-app overlay. This keeps
// the three in the same place and the same shape.
//
// Its lifetime is its own. One wxTimer owned by the frame drives both the fade
// over the last kToastFadeMs (SetTransparent, where the platform can) and the
// Destroy() at the end; no caller holds a pointer to it, and the static stack
// below is left by the destructor.
class ToastWindow : public wxFrame
{
public:
	ToastWindow(const std::string& message, MessageBoxStyle style, int durationMs);
	~ToastWindow() override;

	// Shows without activating, joins the stack and re-flows it.
	void popUp();

	// A toast still up must not keep the application running once the last
	// real window has closed -- wx would otherwise wait for it to expire.
	bool ShouldPreventAppExit() const override { return false; }

private:
	void onTick(wxTimerEvent& event);

	// Lay every live toast out upwards from the anchor's bottom-right corner,
	// oldest in the corner -- run on every arrival and every departure, so an
	// expiring toast lets the rest slide down instead of leaving a hole.
	static void reflow();

	static std::vector<ToastWindow*>& stack();

	int m_durationMs;
	wxLongLong m_shownAt;
	wxTimer m_timer;
};
