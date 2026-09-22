#pragma once

#include <concepts>
#include <string>
#include <vector>

template <typename T>
concept SliderValue = std::same_as<T, int> || std::same_as<T, float>;

template <typename T>
concept SpinBoxValue = SliderValue<T>;

template <typename T>
concept ComboBoxValue = std::same_as<T, int> || std::same_as<T, std::string>;

template <typename T>
concept RadioButtonValue = std::same_as<T, bool> || std::same_as<T, int>;

// A ListBox binds either a single selection or a vector of them -- the bound
// type is what picks single- vs multi-select mode, so there is one widget and
// no mode flag to keep in step with the value.
template <typename T>
concept ListBoxSelection = std::same_as<T, int> || std::same_as<T, std::string>;

template <typename T>
concept ListBoxValue = ListBoxSelection<T>
	|| std::same_as<T, std::vector<int>>
	|| std::same_as<T, std::vector<std::string>>;

template <typename T>
concept MultiSelectListBoxValue = ListBoxValue<T> && !ListBoxSelection<T>;

// A CheckListBox binds the CHECKED SET, and a set is always a vector: unlike
// ListBoxValue there is no single-value spelling to pick a mode with, because
// "one box ticked" is not a different control. int names an item by position,
// std::string by its text -- the same two readings ListBox's multi bindings
// have, decoded by the same helpers.
template <typename T>
concept CheckListValue = std::same_as<T, std::vector<int>>
	|| std::same_as<T, std::vector<std::string>>;

// Element type behind a ListBox binding: the bound type itself when single,
// the vector's element when multi.
template <typename T>
struct ListBoxItemTraits
{
	using Item = T;
};

template <typename T>
struct ListBoxItemTraits<std::vector<T>>
{
	using Item = T;
};

template <ListBoxValue T>
using ListBoxItem = typename ListBoxItemTraits<T>::Item;

// A Table binds a row SELECTION and, exactly like ListBox, the bound type is
// what picks single- vs multi-select -- there is one widget and no mode flag to
// keep in step with the value. Two ways to name a row:
//
//   int / std::vector<int>                 the row's ORIGINAL index
//   std::string / std::vector<std::string> the text of its first (key) column
//
// Original index, never display position: sorting reorders what is on screen,
// and a binding that shifted underneath the caller every time a header was
// clicked would be useless. Index bindings are therefore the robust ones. A key
// binding reads better at the call site, but two rows sharing a column-0 value
// are indistinguishable through it, and editing that column changes which row
// the binding names -- prefer an index binding when column 0 is editable.
template <typename T>
concept TableSelection = std::same_as<T, int> || std::same_as<T, std::string>;

template <typename T>
concept TableValue = TableSelection<T>
	|| std::same_as<T, std::vector<int>>
	|| std::same_as<T, std::vector<std::string>>;

template <typename T>
concept MultiSelectTableValue = TableValue<T> && !TableSelection<T>;

// Element type behind a Table binding: the bound type itself when single, the
// vector's element when multi.
template <typename T>
struct TableKeyTraits
{
	using Key = T;
};

template <typename T>
struct TableKeyTraits<std::vector<T>>
{
	using Key = T;
};

template <TableValue T>
using TableKey = typename TableKeyTraits<T>::Key;

// A TreeView addresses items by PATH -- the item's labels from the root joined
// by '/' -- rather than by index: an index says nothing about where an item
// sits in a tree, and the native item handles (wxTreeItemId, QTreeWidgetItem*)
// are backend types that cannot cross into the public API.
//
// Unlike ListBox, the bound type does NOT pick the selection mode: a tree is
// single-select by default and TreeView::isMultiSelect() widens it. The vector
// binding is simply what gives multi-select somewhere to put the extra paths,
// which is why isMultiSelect() is only offered on it.
template <typename T>
concept TreeViewValue = std::same_as<T, std::string>
	|| std::same_as<T, std::vector<std::string>>;

template <typename T>
concept MultiSelectTreeViewValue = std::same_as<T, std::vector<std::string>>;
