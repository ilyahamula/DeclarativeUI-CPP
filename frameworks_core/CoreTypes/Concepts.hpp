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
