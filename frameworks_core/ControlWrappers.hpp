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
	void createNativeObject(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style);
public:
	ButtonWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {});
	ButtonWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		std::function<void(void*)> onClick = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	std::function<void()> m_onClick;
	std::function<void(void*)> m_onClickWithWidget;
#endif
};

// TextCtrlWrapper -----------------------------------------------------------
class TextCtrlWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, const std::string& value,
		const Position& pos, const Size& size, long style);
public:
	TextCtrlWrapper(ControlWrapper* parent, std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});
	TextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});
	TextCtrlWrapper(ControlWrapper* parent, std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&, void*)> onChange = {});
	TextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_ownedValue;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
#endif
};

// PasswordInputWrapper -----------------------------------------------------------
class PasswordInputWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, const std::string& value,
		const Position& pos, const Size& size, long style);
public:
	PasswordInputWrapper(ControlWrapper* parent, std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});
	PasswordInputWrapper(ControlWrapper* parent, const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});
	PasswordInputWrapper(ControlWrapper* parent, std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&, void*)> onChange = {});
	PasswordInputWrapper(ControlWrapper* parent, const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_ownedValue;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
#endif
};

// MultiLineTextCtrlWrapper -----------------------------------------------------------
class MultiLineTextCtrlWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, const std::string& value,
		const Position& pos, const Size& size, long style);
public:
	MultiLineTextCtrlWrapper(ControlWrapper* parent, std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});
	MultiLineTextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {});
	MultiLineTextCtrlWrapper(ControlWrapper* parent, std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&, void*)> onChange = {});
	MultiLineTextCtrlWrapper(ControlWrapper* parent, const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_ownedValue;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
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
	void createNativeObject(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style);
public:
	ClickableTextWrapper(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {});
	ClickableTextWrapper(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style,
		std::function<void(void*)> onClick = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_text;
	std::function<void()> m_onClick;
	std::function<void(void*)> m_onClickWithWidget;
#endif
};

// LinkTextWrapper -----------------------------------------------------------
class LinkTextWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style);
public:
	LinkTextWrapper(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {});
	LinkTextWrapper(ControlWrapper* parent, const std::string& text,
		const Position& pos, const Size& size, long style,
		std::function<void(void*)> onClick = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_text;
	std::function<void()> m_onClick;
	std::function<void(void*)> m_onClickWithWidget;
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
	void createNativeObject(ControlWrapper* parent, const Date& value,
		const Position& pos, const Size& size, long style);
public:
	DatePickerWrapper(ControlWrapper* parent, Date& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Date&)> onChange = {});
	DatePickerWrapper(ControlWrapper* parent, const Date& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Date&)> onChange = {});
	DatePickerWrapper(ControlWrapper* parent, Date& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Date&, void*)> onChange = {});
	DatePickerWrapper(ControlWrapper* parent, const Date& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Date&, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Date m_ownedValue{};
	std::optional<std::reference_wrapper<Date>> m_externalRef;
	std::function<void(const Date&)> m_onChange;
	std::function<void(const Date&, void*)> m_onChangeWithWidget;
#endif
};

// TimePickerWrapper -----------------------------------------------------------
class TimePickerWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, const Time& value,
		const Position& pos, const Size& size, long style);
public:
	TimePickerWrapper(ControlWrapper* parent, Time& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Time&)> onChange = {});
	TimePickerWrapper(ControlWrapper* parent, const Time& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Time&)> onChange = {});
	TimePickerWrapper(ControlWrapper* parent, Time& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Time&, void*)> onChange = {});
	TimePickerWrapper(ControlWrapper* parent, const Time& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Time&, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Time m_ownedValue{};
	std::optional<std::reference_wrapper<Time>> m_externalRef;
	std::function<void(const Time&)> m_onChange;
	std::function<void(const Time&, void*)> m_onChangeWithWidget;
#endif
};

// SliderWrapper -----------------------------------------------------------
template <SliderValue T>
class SliderWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, Range<T> range, const T& value,
		const Position& pos, const Size& size, long style);
public:
	SliderWrapper(ControlWrapper* parent, Range<T> range, T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});
	SliderWrapper(ControlWrapper* parent, Range<T> range, const T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});
	SliderWrapper(ControlWrapper* parent, Range<T> range, T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T, void*)> onChange = {});
	SliderWrapper(ControlWrapper* parent, Range<T> range, const T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Range<T> m_range;
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
	std::function<void(T, void*)> m_onChangeWithWidget;
#endif
};

extern template class SliderWrapper<int>;
extern template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------
template <SpinBoxValue T>
class SpinBoxWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, Range<T> range, const T& value,
		const Position& pos, const Size& size, long style);
public:
	SpinBoxWrapper(ControlWrapper* parent, Range<T> range, T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});
	SpinBoxWrapper(ControlWrapper* parent, Range<T> range, const T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});
	SpinBoxWrapper(ControlWrapper* parent, Range<T> range, T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T, void*)> onChange = {});
	SpinBoxWrapper(ControlWrapper* parent, Range<T> range, const T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Range<T> m_range;
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
	std::function<void(T, void*)> m_onChangeWithWidget;
#endif
};

extern template class SpinBoxWrapper<int>;
extern template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------
template <RadioButtonValue T>
class RadioButtonWrapper : public ControlWrapper
{
	// Returns the radio's group index (0 for bool); standalone = each its own group.
	int createNativeObject(ControlWrapper* parent, const std::string& label,
		const T& value, const Position& pos, const Size& size, long style, bool standalone);
public:
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		T& value, const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		const T& initialValue, const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {});
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		T& value, const Position& pos, const Size& size, long style,
		std::function<void(T, void*)> onChange = {});
	RadioButtonWrapper(ControlWrapper* parent, const std::string& label,
		const T& initialValue, const Position& pos, const Size& size, long style,
		std::function<void(T, void*)> onChange = {});

#ifdef USE_IMGUI
	static void resetGroupId() { s_radioButtonId = 0; s_lastGroup = nullptr; }
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	T m_ownedValue;
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
	std::function<void(T, void*)> m_onChangeWithWidget;
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
	void createNativeObject(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style, const bool& checked);
public:
	CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		bool& checked, std::function<void(bool)> onChange = {});
	CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		const bool& initialChecked, std::function<void(bool)> onChange = {});
	CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		bool& checked, std::function<void(bool, void*)> onChange = {});
	CheckBoxWrapper(ControlWrapper* parent, const std::string& label,
		const Position& pos, const Size& size, long style,
		const bool& initialChecked, std::function<void(bool, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	bool m_ownedValue;
	std::optional<std::reference_wrapper<bool>> m_externalRef;
	std::function<void(bool)> m_onChange;
	std::function<void(bool, void*)> m_onChangeWithWidget;
#endif
};

// ToggleButtonWrapper -----------------------------------------------------------
class ToggleButtonWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, const std::string& label,
		const bool& toggled, const Position& pos, const Size& size, long style);
public:
	ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
		bool& toggled, const Position& pos, const Size& size, long style,
		std::function<void(bool)> onChange = {});
	ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
		const bool& initialToggled, const Position& pos, const Size& size, long style,
		std::function<void(bool)> onChange = {});
	ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
		bool& toggled, const Position& pos, const Size& size, long style,
		std::function<void(bool, void*)> onChange = {});
	ToggleButtonWrapper(ControlWrapper* parent, const std::string& label,
		const bool& initialToggled, const Position& pos, const Size& size, long style,
		std::function<void(bool, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_label;
	bool m_ownedValue;
	std::optional<std::reference_wrapper<bool>> m_externalRef;
	std::function<void(bool)> m_onChange;
	std::function<void(bool, void*)> m_onChangeWithWidget;
#endif
};

// ImageWrapper -----------------------------------------------------------
class ImageWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, const std::string& filePath,
		const Position& pos, const Size& size, long style);
public:
	ImageWrapper(ControlWrapper* parent, const std::string& filePath,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {},
		std::function<void()> onHover = {});
	ImageWrapper(ControlWrapper* parent, const std::string& filePath,
		const Position& pos, const Size& size, long style,
		std::function<void(void*)> onClick,
		std::function<void(void*)> onHover = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_filePath;
	void* m_textureId = nullptr; // ImTextureID (void*) holding the GL texture handle
	int m_imgWidth = 0;
	int m_imgHeight = 0;
	int m_displayWidth = -1;
	int m_displayHeight = -1;
	std::function<void()> m_onClick;
	std::function<void()> m_onHover;
	std::function<void(void*)> m_onClickWithWidget;
	std::function<void(void*)> m_onHoverWithWidget;
#endif
};

// ColorPickerWrapper -----------------------------------------------------------
class ColorPickerWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, const Color& value,
		const Position& pos, const Size& size, long style);
public:
	ColorPickerWrapper(ControlWrapper* parent, Color& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Color&)> onChange = {});
	ColorPickerWrapper(ControlWrapper* parent, const Color& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Color&)> onChange = {});
	ColorPickerWrapper(ControlWrapper* parent, Color& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Color&, void*)> onChange = {});
	ColorPickerWrapper(ControlWrapper* parent, const Color& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Color&, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	Color m_ownedValue;
	std::optional<std::reference_wrapper<Color>> m_externalRef;
	std::function<void(const Color&)> m_onChange;
	std::function<void(const Color&, void*)> m_onChangeWithWidget;
#endif
};

// SeparatorWrapper -----------------------------------------------------------
class SeparatorWrapper : public ControlWrapper
{
public:
	SeparatorWrapper(ControlWrapper* parent,
		const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
#endif
};

// ProgressBarWrapper -----------------------------------------------------------
class ProgressBarWrapper : public ControlWrapper
{
public:
	ProgressBarWrapper(ControlWrapper* parent, const float& value,
		const Position& pos, const Size& size, long style);
	ProgressBarWrapper(ControlWrapper* parent, float& value,
		const Position& pos, const Size& size, long style);

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	float m_ownedValue;
	std::optional<std::reference_wrapper<float>> m_externalRef;
#endif
};

// ComboBoxWrapper -----------------------------------------------------------
template <ComboBoxValue T>
class ComboBoxWrapper : public ControlWrapper
{
	void createNativeObject(ControlWrapper* parent, const std::vector<std::string>& choices,
		const T& selected, const Position& pos, const Size& size, long style);
public:
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		T& selected, const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {});
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		const T& selected, const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {});
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		T& selected, const Position& pos, const Size& size, long style,
		std::function<void(const T&, void*)> onChange = {});
	ComboBoxWrapper(ControlWrapper* parent, std::vector<std::string> choices,
		const T& selected, const Position& pos, const Size& size, long style,
		std::function<void(const T&, void*)> onChange = {});

#ifdef USE_IMGUI
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags) override;
private:
	std::string m_items;
	std::vector<std::string> m_choices;
	int m_currentItem = 0;
	T m_ownedSelected{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
#endif
};

extern template class ComboBoxWrapper<std::string>;
extern template class ComboBoxWrapper<int>;
