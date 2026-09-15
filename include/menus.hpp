#pragma once

#include "frameworks_core/CoreTypes/MenuModel.hpp"

#include <concepts>
#include <utility>

// The public spelling of the menu model. MenuItem, Menu and Shortcut are
// already framework-independent data with fluent modifiers, so this header is
// the name callers write rather than a second layer over them:
//
//   MenuBar { {
//       Menu { "File", {
//           MenuItem{"New"}.withShortcut("Ctrl+N").onSelect(...),
//           MenuItem::Separator(),
//           MenuItem{"Recent"}.withSubmenu({ MenuItem{"a.cpp"}, MenuItem{"b.cpp"} }),
//       } },
//       Menu { "View", {
//           MenuItem{"Word wrap"}.withShortcut("Ctrl+Shift+W").checkable(wordWrap),
//       } },
//   } }
using MenuBar = MenuBarModel;

// A top-level window a menu bar can be attached to. Today that is Window and
// only Window: wxMenuBar attaches to a wxFrame and nothing else, which is the
// reason Window exists at all -- so this is the concept that says so, rather
// than a promise Dialog would quietly fail to keep.
template <typename T>
concept MenuBarHost = requires(T element, MenuBar bar) {
	{ element.withMenuBar(std::move(bar)) } -> std::same_as<T&>;
};

// A leaf that can carry a right-click menu. Leaf-only, like withTooltip: the
// modifier lives on Widget<W>, so a container simply does not have it and
// asking for one is a compile error rather than a silently ignored call.
template <typename T>
concept ContextMenuHost = requires(T element, ContextMenu menu) {
	{ element.withContextMenu(std::move(menu)) } -> std::same_as<T&>;
};
