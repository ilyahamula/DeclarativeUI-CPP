#pragma once

#include "ControlWrapper.hpp"
#include "create_and_add.hpp"

#include <cstdio>
#include <functional>
#include <optional>
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
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});
	TextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_ownedValue;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
#endif
};

// PasswordInputWrapper -----------------------------------------------------------
class PasswordInputWrapper : public ControlWrapper
{
public:
	PasswordInputWrapper(ControlWrapper* parent, std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});
	PasswordInputWrapper(ControlWrapper* parent, const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_ownedValue;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
#endif
};

// MultiLineTextCtrlWrapper -----------------------------------------------------------
class MultiLineTextCtrlWrapper : public ControlWrapper
{
public:
	MultiLineTextCtrlWrapper(ControlWrapper* parent, std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});
	MultiLineTextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_ownedValue;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
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

// DatePickerWrapper -----------------------------------------------------------
class DatePickerWrapper : public ControlWrapper
{
public:
	DatePickerWrapper(ControlWrapper* parent, Date& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Date&)> onChange = {});
	DatePickerWrapper(ControlWrapper* parent, const Date& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Date&)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Date m_ownedValue{};
	std::optional<std::reference_wrapper<Date>> m_externalRef;
	std::function<void(const Date&)> m_onChange;
#endif
};

// TimePickerWrapper -----------------------------------------------------------
class TimePickerWrapper : public ControlWrapper
{
public:
	TimePickerWrapper(ControlWrapper* parent, Time& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Time&)> onChange = {});
	TimePickerWrapper(ControlWrapper* parent, const Time& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Time&)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Time m_ownedValue{};
	std::optional<std::reference_wrapper<Time>> m_externalRef;
	std::function<void(const Time&)> m_onChange;
#endif
};

// SliderWrapper -----------------------------------------------------------
template <SliderValue T>
class SliderWrapper : public ControlWrapper
{
public:
	SliderWrapper(ControlWrapper* parent, Range<T> range, T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});
	SliderWrapper(ControlWrapper* parent, Range<T> range, const T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Range<T> m_range;
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
#endif
};

extern template class SliderWrapper<int>;
extern template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------
template <SpinBoxValue T>
class SpinBoxWrapper : public ControlWrapper
{
public:
	SpinBoxWrapper(ControlWrapper* parent, Range<T> range, T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});
	SpinBoxWrapper(ControlWrapper* parent, Range<T> range, const T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Range<T> m_range;
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
#endif
};

extern template class SpinBoxWrapper<int>;
extern template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------
template <RadioButtonValue T>
class RadioButtonWrapper : public ControlWrapper
{
public:
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		T& value, const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		const T& initialValue, const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});

#ifdef USE_IMGUI
	static void resetGroupId() { s_radioButtonId = 0; s_lastGroup = nullptr; }
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	T m_ownedValue;
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
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
		bool& checked, std::function<void(bool)> onChange = {});
	CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		const bool& initialChecked, std::function<void(bool)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	bool m_ownedValue;
	std::optional<std::reference_wrapper<bool>> m_externalRef;
	std::function<void(bool)> m_onChange;
#endif
};

// ToggleButtonWrapper -----------------------------------------------------------
class ToggleButtonWrapper : public ControlWrapper
{
public:
	ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
		bool& toggled, const Position& pos, const Size& size, long style,
		std::function<void(bool)> onChange = {});
	ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
		const bool& initialToggled, const Position& pos, const Size& size, long style,
		std::function<void(bool)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	bool m_ownedValue;
	std::optional<std::reference_wrapper<bool>> m_externalRef;
	std::function<void(bool)> m_onChange;
#endif
};

// ComboBoxWrapper -----------------------------------------------------------
template <ComboBoxValue T>
class ComboBoxWrapper : public ControlWrapper
{
public:
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		T& selected, const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {});
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		const T& selected, const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_items;
	std::vector<std::string> m_choices;
	int m_currentItem = 0;
	T m_ownedSelected{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(const T&)> m_onChange;
#endif
};

extern template class ComboBoxWrapper<std::string>;
extern template class ComboBoxWrapper<int>;
