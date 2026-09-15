#pragma once

#include "frameworks_core/CoreTypes/BoundValue.hpp"
#include "frameworks_core/CoreTypes/Shortcut.hpp"

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// The framework-independent menu model, like TreeItem and TableColumn: plain
// data every backend walks, never a native menu. wx builds a wxMenuBar from it,
// Qt a QMenuBar, ImGui draws it -- all three from these same fields.
//
// A MenuItem carries real constructors rather than aggregate initialisation
// because BoundValue's constructors are explicit: brace-initialising a member
// would never pick the binding overload, which is the whole point of
// checkable(bool&) vs checkable(const bool&).
struct MenuItem
{
	explicit MenuItem(std::string itemLabel)
		: label(std::move(itemLabel))
	{
	}

	// A horizontal rule between groups of commands. It carries no label and no
	// handler, so it is the one MenuItem that is not built from a label.
	static MenuItem Separator()
	{
		MenuItem item{ std::string() };
		item.isSeparator = true;
		return item;
	}

	MenuItem& onSelect(std::function<void()> callback)
	{
		selectHandler = std::move(callback);
		return *this;
	}

	// Parsed once, here, so the backends never see the caller's text. Text we
	// cannot name a key in is kept for display and registers no accelerator --
	// a shortcut that silently does nothing is better than a menu that refuses
	// to open (see the backends' logger lines).
	MenuItem& withShortcut(std::string text)
	{
		shortcut = Shortcut::parse(text);
		shortcutText = shortcut ? shortcut->display() : std::move(text);
		return *this;
	}

	// A check mark the framework owns...
	MenuItem& checkable(const bool& value)
	{
		checkedFlag = BoundValue<bool>(value);
		return *this;
	}

	// ...or one that IS the caller's bool: activating the item writes through,
	// and anything else writing it moves the check mark.
	MenuItem& checkable(bool& value)
	{
		checkedFlag = BoundValue<bool>(value);
		return *this;
	}

	MenuItem& isDisabled(const bool& value)
	{
		disabledFlag = DisabledFlag(value);
		return *this;
	}

	MenuItem& isDisabled(bool& value)
	{
		disabledFlag = DisabledFlag(value);
		return *this;
	}

	// A submenu nests to any depth. An item with a submenu is a container: its
	// own onSelect never fires, exactly as on every desktop platform.
	MenuItem& withSubmenu(std::vector<MenuItem> items)
	{
		submenu = std::move(items);
		return *this;
	}

	std::string label;
	std::function<void()> selectHandler;
	std::optional<Shortcut> shortcut;
	std::string shortcutText;
	std::optional<BoundValue<bool>> checkedFlag; // engaged => a check item
	DisabledFlag disabledFlag { false };
	std::vector<MenuItem> submenu;
	bool isSeparator = false;
};

// One top-level menu ("File") and its items.
struct Menu
{
	Menu(std::string menuLabel, std::vector<MenuItem> menuItems)
		: label(std::move(menuLabel))
		, items(std::move(menuItems))
	{
	}

	std::string label;
	std::vector<MenuItem> items;
};

// The bar itself. Window::withMenuBar() takes one by value and the backend
// copies it, because the Window that built it is a temporary that dies with
// the show() expression -- the same reason every wrapper copies its data.
struct MenuBarModel
{
	MenuBarModel() = default;

	MenuBarModel(std::vector<Menu> barMenus)
		: menus(std::move(barMenus))
	{
	}

	bool empty() const { return menus.empty(); }

	std::vector<Menu> menus;
};

// A right-click menu is the same items with no bar above them (T2.3).
using ContextMenu = std::vector<MenuItem>;
