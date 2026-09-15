#pragma once

#include "frameworks_core/CoreTypes/MenuModel.hpp"

#include <string>

// Drawing the framework-independent menu model with ImGui. There is no native
// menu here to attach anything to, so the model IS the menu: it is walked from
// scratch every frame, exactly like the rest of the tree.
//
// Split in two because the two halves run in different places. A menu's items
// are only drawn while its popup is open, but its shortcut has to work whether
// or not anyone has opened it -- so routing is a separate pass over the whole
// tree, run once inside the window's Begin/End scope.
//
// drawMenuItems() is the piece T2.3's context menus reuse: the same items, in
// a popup with no bar above them.

// Draws items into the CURRENT menu scope (a BeginMenu popup, or a context
// popup). `path` disambiguates unbound check state -- see MenuDraw.cpp.
void drawMenuItems(const std::vector<MenuItem>& items, const std::string& path = {});

// Evaluates every shortcut in the tree, submenus included, and activates the
// item that fires. Call inside the owning window's Begin/End: shortcuts are
// routed with ImGuiInputFlags_RouteFocused, so they fire while that window is
// the active one and not while a different window has focus -- which is what
// a wx accelerator table and a Qt QAction do, and what the caller expects from
// one shortcut string on three backends.
void routeMenuShortcuts(const std::vector<MenuItem>& items, const std::string& path = {});

// The bar itself: BeginMenuBar over the model, then the routing pass. The
// window must carry ImGuiWindowFlags_MenuBar.
void drawMenuBar(const MenuBarModel& model);
