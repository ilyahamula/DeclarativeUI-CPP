#pragma once

#include "frameworks_core/CoreTypes/MenuModel.hpp"

class wxFrame;
class wxWindow;
struct wxPoint;

// Builds a wxMenuBar from the framework-independent model and attaches it to
// the frame -- the one thing a wxDialog cannot be given, and the reason Window
// exists (include/window.hpp).
//
// The model is COPIED here and the copy lives as long as the frame does. It has
// to: the Window that built it is a temporary that dies with the show()
// expression, whereas a menu is not walked until the user opens one. Bound
// check and disabled flags belong to the caller and are polled through the
// ordinary RefSync shape, so an item follows a bool written from anywhere.
//
// The menu bar is native chrome outside the frame's client area on every
// platform, so the layout engine is never told about it.
void attachMenuBar(wxFrame* frame, const MenuBarModel& model);

// Pops a right-click menu up over `owner` at `pos` (client coordinates) and
// runs the chosen item's handler before returning -- Widget<W>::withContextMenu.
//
// Unlike the bar, a popup is built from the model EVERY time it opens and thrown
// away again, so bound check and disabled flags are simply read here and nothing
// is polled. Selection comes back through wxWindow::GetPopupMenuSelectionFromUser
// rather than a menu event, which keeps the whole interaction inside this call
// and leaves no handler bound to a window that outlives the menu.
//
// A shortcut on a context-menu item is DISPLAYED but not registered: the menu is
// not attached to a frame, so wx builds no accelerator from it. Qt and ImGui do
// the same, so the three agree.
// `items` is the backend's OWN copy of the model (the wrapper that supplied it
// is never captured, per the wrapper contract), and is non-const because an
// unbound check item's state lives in it -- a bound one writes through to the
// caller's bool either way.
void popupContextMenu(wxWindow* owner, ContextMenu& items, const wxPoint& pos);
