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
	SliderWrapper(ControlWrapper* parent, Range<T> range, T* ptrValue,
		const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
#endif

private:
	Range<T> m_range;
	T* m_ptrValue = nullptr;
};

extern template class SliderWrapper<int>;
extern template class SliderWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------
class RadioButtonWrapper : public ControlWrapper
{
public:
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
#endif

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

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
#endif

private:
	bool m_checked = false;
};

// ComboBoxWrapper -----------------------------------------------------------
class ComboBoxWrapper : public ControlWrapper
{
public:
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		const std::string& selected, const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_items;
#endif

private:
	int m_currentItem = 0;
};
