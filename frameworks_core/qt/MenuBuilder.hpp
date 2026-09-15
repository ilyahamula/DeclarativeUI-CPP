#pragma once

#include "frameworks_core/CoreTypes/MenuModel.hpp"

class QMainWindow;

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
