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

private:
	char m_buf[256] = {};
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

private:
	int m_value = 0;
};

// RadioButtonWrapper -----------------------------------------------------------
class RadioButtonWrapper : public ControlWrapper
{
public:
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style);

private:
	bool m_active = false;
};

// CheckBoxWrapper -----------------------------------------------------------
class CheckBoxWrapper : public ControlWrapper
{
public:
	CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		bool checked = false);

private:
	bool m_checked = false;
};

// ComboBoxWrapper -----------------------------------------------------------
class ComboBoxWrapper : public ControlWrapper
{
public:
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		const std::string& selected, const Position& pos, const Size& size, long style);

private:
	int m_currentItem = 0;
};
