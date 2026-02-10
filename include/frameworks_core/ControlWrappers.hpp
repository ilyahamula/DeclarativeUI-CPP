#pragma once

#include "ControlWrapper.hpp"

#include <functional>
#include <string>
#include <vector>

// ButtonWrapper -----------------------------------------------------------
class ButtonWrapper : public ControlWrapper
{
public:
	ButtonWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {});
};

// TextCtrlWrapper -----------------------------------------------------------
class TextCtrlWrapper : public ControlWrapper
{
public:
	TextCtrlWrapper(ControlWrapper* parent, const std::string& value,
		const Position& pos, const Size& size, long style);
};

// StaticTextWrapper -----------------------------------------------------------
class StaticTextWrapper : public ControlWrapper
{
public:
	StaticTextWrapper(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style);
};

// SliderWrapper -----------------------------------------------------------
class SliderWrapper : public ControlWrapper
{
public:
	SliderWrapper(ControlWrapper* parent, Range range,
		const Position& pos, const Size& size, long style);
};

// RadioButtonWrapper -----------------------------------------------------------
class RadioButtonWrapper : public ControlWrapper
{
public:
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style);
};

// CheckBoxWrapper -----------------------------------------------------------
class CheckBoxWrapper : public ControlWrapper
{
public:
	CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		bool checked = false);
};

// ComboBoxWrapper -----------------------------------------------------------
class ComboBoxWrapper : public ControlWrapper
{
public:
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		const std::string& selected, const Position& pos, const Size& size, long style);
};
