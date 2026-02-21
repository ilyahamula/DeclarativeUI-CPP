#pragma once

#include "ControlWrapper.hpp"
#include "create_and_add.hpp"

#include <cstdio>
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

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	std::function<void()> m_onClick;
#endif
};

// TextCtrlWrapper -----------------------------------------------------------
class TextCtrlWrapper : public ControlWrapper
{
public:
	TextCtrlWrapper(ControlWrapper* parent, std::string& value,
		const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string& m_value;
#endif
};

// ReadonlyTextCtrlWrapper -----------------------------------------------------------
class ReadonlyTextCtrlWrapper : public ControlWrapper
{
public:
	ReadonlyTextCtrlWrapper(ControlWrapper* parent, const std::string& value,
		const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	const std::string& m_value;
#endif
};

// ClickableTextWrapper -----------------------------------------------------------
class ClickableTextWrapper : public ControlWrapper
{
public:
	ClickableTextWrapper(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_text;
	std::function<void()> m_onClick;
#endif
};

// LinkTextWrapper -----------------------------------------------------------
class LinkTextWrapper : public ControlWrapper
{
public:
	LinkTextWrapper(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_text;
	std::function<void()> m_onClick;
#endif
};

// StaticTextWrapper -----------------------------------------------------------
class StaticTextWrapper : public ControlWrapper
{
public:
	StaticTextWrapper(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_text;
#endif
};

// SliderWrapper -----------------------------------------------------------
template <SliderValue T>
class SliderWrapper : public ControlWrapper
{
public:
	SliderWrapper(ControlWrapper* parent, Range<T> range, T& value,
		const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Range<T> m_range;
	T& m_value;
#endif
};

extern template class SliderWrapper<int>;
extern template class SliderWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------
template <RadioButtonValue T>
class RadioButtonWrapper : public ControlWrapper
{
public:
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		T& value, const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	static void resetGroupId() { s_radioButtonId = 0; s_lastGroup = nullptr; }
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	T& m_value;
	int m_index = 0;
#endif

private:
	static inline int s_radioButtonId = 0;
	static inline int* s_lastGroup = nullptr;
};

extern template class RadioButtonWrapper<bool>;
extern template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------
class CheckBoxWrapper : public ControlWrapper
{
public:
	CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		bool& checked);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	bool& m_checked;
#endif
};

// ComboBoxWrapper -----------------------------------------------------------
template <ComboBoxValue T>
class ComboBoxWrapper : public ControlWrapper
{
public:
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		T& selected, const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_items;
	std::vector<std::string> m_choices;
	int m_currentItem = 0;
	T& m_selected = nullptr;
#endif
};

extern template class ComboBoxWrapper<std::string>;
extern template class ComboBoxWrapper<int>;
