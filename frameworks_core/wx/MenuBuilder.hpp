#pragma once

#include "frameworks_core/CoreTypes/MenuModel.hpp"

class wxFrame;

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
