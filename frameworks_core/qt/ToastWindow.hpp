#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <QWidget>

#include <string>
#include <vector>

// The native half of ToastWrapper on Qt: a frameless, always-on-top top-level
// that never takes focus, placed at the bottom-right of the active window.
//
// Qt::ToolTip is what keeps it out of the way twice over: the window manager
// never activates it, and QApplication does not count it as a window that keeps
// the application alive, so a toast still up when the last real window closes
// does not hold quitOnLastWindowClosed hostage.
//
// Its lifetime is its own: a QTimer starts the fade, a QPropertyAnimation on
// windowOpacity runs it, and close() + WA_DeleteOnClose end it. No caller holds
// a pointer to it -- the only list that does is the static stack below, which
// the destructor leaves.
class ToastWindow : public QWidget
{
public:
	ToastWindow(const std::string& message, MessageBoxStyle style, int durationMs);
	~ToastWindow() override;

	// Shows without activating, joins the stack and re-flows it.
	void popUp();

private:
	// Lay every live toast out upwards from the anchor's bottom-right corner,
	// oldest in the corner -- run on every arrival and every departure, so an
	// expiring toast lets the rest slide down instead of leaving a hole.
	static void reflow();

	static std::vector<ToastWindow*>& stack();

	int m_durationMs;
};
