#pragma once

#include "frameworks_core/CoreTypes/MenuModel.hpp"

class QMainWindow;
class QPoint;
class QWidget;

// Builds a QMenuBar from the framework-independent model and attaches it to the
// window -- the Qt twin of frameworks_core/wx/MenuBuilder.hpp, same contract.
//
// The model is COPIED here and the copy lives as long as the window does: the
// Window that built it is a temporary that dies with the show() expression,
// whereas a menu is not walked until the user opens one. Bound check and
// disabled flags belong to the caller and are polled through the ordinary
// RefSync shape.
//
// QMainWindow stacks the bar above its central widget, and the engine lays out
// into that central widget -- so the window has to grow by the bar's height for
// the content to keep the space it measured. That height is what this returns,
// and it is 0 on macOS, where the bar goes to the system menu and costs the
// window nothing. Returns 0 for an empty model (no bar is attached at all).
int attachMenuBar(QMainWindow* window, const MenuBarModel& model);

// Pops a right-click menu up over `owner` at `pos` (widget coordinates) and runs
// the chosen item's handler before returning -- Widget<W>::withContextMenu, and
// the Qt twin of the wx entry point of the same name.
//
// Unlike the bar, a popup is built from the model EVERY time it opens and thrown
// away again, so bound check and disabled flags are simply read here and nothing
// is polled. QMenu::exec is synchronous and hands back the chosen QAction, so the
// whole interaction stays inside this call and no signal outlives the menu.
//
// `items` is the backend's OWN copy of the model (the wrapper that supplied it
// is never captured), and is non-const because an unbound check item's state
// lives in it -- a bound one writes through to the caller's bool either way.
//
// A shortcut on a context-menu item is DISPLAYED but not registered: the action
// lives only as long as the menu. wx and ImGui do the same, so the three agree.
void popupContextMenu(QWidget* owner, ContextMenu& items, const QPoint& pos);
