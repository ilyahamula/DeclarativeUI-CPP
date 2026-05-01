#pragma once

#include <concepts>
#include <string>

template <typename T>
concept SliderValue = std::same_as<T, int> || std::same_as<T, float>;

template <typename T>
concept SpinBoxValue = SliderValue<T>;

template <typename T>
concept ComboBoxValue = std::same_as<T, int> || std::same_as<T, std::string>;

template <typename T>
concept RadioButtonValue = std::same_as<T, bool> || std::same_as<T, int>;
