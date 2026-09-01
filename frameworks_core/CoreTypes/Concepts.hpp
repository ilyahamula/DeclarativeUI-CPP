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
