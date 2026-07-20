#pragma once

#include "ControlWrapper.hpp"

#include <cstdio>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// NOTE: All wrappers follow the same contract:
//  - The constructor only collects the data needed to build the control
//    (label/value/range/callbacks, plus position/size/style which are stored
//    in the ControlWrapper base). It does NOT create any native object.
//  - realize() creates the native control on retained backends; on ImGui
//    the widget draws itself in render() at the engine-computed rect.
// Because the constructors are pure data collection, they are shared by every
// backend and defined inline here.

// ButtonWrapper -----------------------------------------------------------
class ButtonWrapper : public ControlWrapper
{
public:
	ButtonWrapper(const std::string& label,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {},
		std::function<void(void*)> onClickWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_onClick(std::move(onClick))
		, m_onClickWithWidget(std::move(onClickWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	std::string m_label;
	std::function<void()> m_onClick;
	std::function<void(void*)> m_onClickWithWidget;
};

// TextCtrlWrapper -----------------------------------------------------------
class TextCtrlWrapper : public ControlWrapper
{
public:
	TextCtrlWrapper(std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {},
		std::function<void(const std::string&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	TextCtrlWrapper(const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {},
		std::function<void(const std::string&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(initialValue)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	std::string m_ownedValue;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
};

// PasswordInputWrapper -----------------------------------------------------------
class PasswordInputWrapper : public ControlWrapper
{
public:
	PasswordInputWrapper(std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {},
		std::function<void(const std::string&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	PasswordInputWrapper(const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {},
		std::function<void(const std::string&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(initialValue)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	std::string m_ownedValue;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
};

// MultiLineTextCtrlWrapper -----------------------------------------------------------
class MultiLineTextCtrlWrapper : public ControlWrapper
{
public:
	MultiLineTextCtrlWrapper(std::string& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {},
		std::function<void(const std::string&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	MultiLineTextCtrlWrapper(const std::string& initialValue,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {},
		std::function<void(const std::string&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(initialValue)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	std::string m_ownedValue;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
};

// ReadonlyTextCtrlWrapper -----------------------------------------------------------
class ReadonlyTextCtrlWrapper : public ControlWrapper
{
public:
	ReadonlyTextCtrlWrapper(const std::string& value,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_value(value)
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	const std::string& m_value;
};

// ClickableTextWrapper -----------------------------------------------------------
class ClickableTextWrapper : public ControlWrapper
{
public:
	ClickableTextWrapper(const std::string& text,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {},
		std::function<void(void*)> onClickWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_text(text)
		, m_onClick(std::move(onClick))
		, m_onClickWithWidget(std::move(onClickWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	std::string m_text;
	std::function<void()> m_onClick;
	std::function<void(void*)> m_onClickWithWidget;
};

// LinkTextWrapper -----------------------------------------------------------
class LinkTextWrapper : public ControlWrapper
{
public:
	LinkTextWrapper(const std::string& text,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {},
		std::function<void(void*)> onClickWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_text(text)
		, m_onClick(std::move(onClick))
		, m_onClickWithWidget(std::move(onClickWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	std::string m_text;
	std::function<void()> m_onClick;
	std::function<void(void*)> m_onClickWithWidget;
};

// StaticTextWrapper -----------------------------------------------------------
class StaticTextWrapper : public ControlWrapper
{
public:
	StaticTextWrapper(const std::string& text,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_text(text)
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	std::string m_text;
};

// DatePickerWrapper -----------------------------------------------------------
class DatePickerWrapper : public ControlWrapper
{
public:
	DatePickerWrapper(Date& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Date&)> onChange = {},
		std::function<void(const Date&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	DatePickerWrapper(const Date& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Date&)> onChange = {},
		std::function<void(const Date&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	Date m_ownedValue{};
	std::optional<std::reference_wrapper<Date>> m_externalRef;
	std::function<void(const Date&)> m_onChange;
	std::function<void(const Date&, void*)> m_onChangeWithWidget;
};

// TimePickerWrapper -----------------------------------------------------------
class TimePickerWrapper : public ControlWrapper
{
public:
	TimePickerWrapper(Time& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Time&)> onChange = {},
		std::function<void(const Time&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	TimePickerWrapper(const Time& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Time&)> onChange = {},
		std::function<void(const Time&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	Time m_ownedValue{};
	std::optional<std::reference_wrapper<Time>> m_externalRef;
	std::function<void(const Time&)> m_onChange;
	std::function<void(const Time&, void*)> m_onChangeWithWidget;
};

// SliderWrapper -----------------------------------------------------------
template <SliderValue T>
class SliderWrapper : public ControlWrapper
{
public:
	SliderWrapper(Range<T> range, T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {},
		std::function<void(T, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_range(range)
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	SliderWrapper(Range<T> range, const T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {},
		std::function<void(T, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_range(range)
		, m_ownedValue(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	Range<T> m_range;
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
	std::function<void(T, void*)> m_onChangeWithWidget;
};

extern template class SliderWrapper<int>;
extern template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------
template <SpinBoxValue T>
class SpinBoxWrapper : public ControlWrapper
{
public:
	SpinBoxWrapper(Range<T> range, T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {},
		std::function<void(T, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_range(range)
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	SpinBoxWrapper(Range<T> range, const T& value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {},
		std::function<void(T, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_range(range)
		, m_ownedValue(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	Range<T> m_range;
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
	std::function<void(T, void*)> m_onChangeWithWidget;
};

extern template class SpinBoxWrapper<int>;
extern template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------
template <RadioButtonValue T>
class RadioButtonWrapper : public ControlWrapper
{
public:
	RadioButtonWrapper(const std::string& label,
		T& value, const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {},
		std::function<void(T, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
		assignGroupIndex(&value);
	}
	RadioButtonWrapper(const std::string& label,
		const T& initialValue, const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {},
		std::function<void(T, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_ownedValue(initialValue)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
		assignGroupIndex(&m_ownedValue);
	}

	static void resetGroupId() { s_radioButtonId = 0; s_lastGroup = nullptr; }
#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	// Assigns the radio's index within its group. Consecutive radios sharing
	// the same backing value form one group; a distinct address starts a new
	// group (so standalone radios, each backed by their own owned value, end
	// up as independent single-element groups).
	void assignGroupIndex(const T* groupKey)
	{
		if constexpr (std::is_same_v<T, int>)
		{
			if (const_cast<int*>(groupKey) != s_lastGroup)
			{
				s_radioButtonId = 0;
				s_lastGroup = const_cast<int*>(groupKey);
			}
			m_index = s_radioButtonId++;
		}
	}

	std::string m_label;
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
	std::function<void(T, void*)> m_onChangeWithWidget;
	int m_index = 0;

	static inline int s_radioButtonId = 0;
	static inline int* s_lastGroup = nullptr;
};

extern template class RadioButtonWrapper<bool>;
extern template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------
class CheckBoxWrapper : public ControlWrapper
{
public:
	CheckBoxWrapper(const std::string& label,
		const Position& pos, const Size& size, long style,
		bool& checked,
		std::function<void(bool)> onChange = {},
		std::function<void(bool, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_ownedValue(checked)
		, m_externalRef(checked)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	CheckBoxWrapper(const std::string& label,
		const Position& pos, const Size& size, long style,
		const bool& initialChecked,
		std::function<void(bool)> onChange = {},
		std::function<void(bool, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_ownedValue(initialChecked)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	std::string m_label;
	bool m_ownedValue;
	std::optional<std::reference_wrapper<bool>> m_externalRef;
	std::function<void(bool)> m_onChange;
	std::function<void(bool, void*)> m_onChangeWithWidget;
};

// ToggleButtonWrapper -----------------------------------------------------------
class ToggleButtonWrapper : public ControlWrapper
{
public:
	ToggleButtonWrapper(const std::string& label,
		bool& toggled, const Position& pos, const Size& size, long style,
		std::function<void(bool)> onChange = {},
		std::function<void(bool, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_ownedValue(toggled)
		, m_externalRef(toggled)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	ToggleButtonWrapper(const std::string& label,
		const bool& initialToggled, const Position& pos, const Size& size, long style,
		std::function<void(bool)> onChange = {},
		std::function<void(bool, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_ownedValue(initialToggled)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	std::string m_label;
	bool m_ownedValue;
	std::optional<std::reference_wrapper<bool>> m_externalRef;
	std::function<void(bool)> m_onChange;
	std::function<void(bool, void*)> m_onChangeWithWidget;
};

// ImageWrapper -----------------------------------------------------------
class ImageWrapper : public ControlWrapper
{
public:
	ImageWrapper(const std::string& filePath,
		const Position& pos, const Size& size, long style,
		std::function<void()> onClick = {},
		std::function<void(void*)> onClickWithWidget = {},
		std::function<void()> onHover = {},
		std::function<void(void*)> onHoverWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_filePath(filePath)
		, m_displayWidth(size.width)
		, m_displayHeight(size.height)
		, m_onClick(std::move(onClick))
		, m_onClickWithWidget(std::move(onClickWithWidget))
		, m_onHover(std::move(onHover))
		, m_onHoverWithWidget(std::move(onHoverWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

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
};

// ColorPickerWrapper -----------------------------------------------------------
class ColorPickerWrapper : public ControlWrapper
{
public:
	ColorPickerWrapper(Color& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Color&)> onChange = {},
		std::function<void(const Color&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}
	ColorPickerWrapper(const Color& value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Color&)> onChange = {},
		std::function<void(const Color&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	Color m_ownedValue;
	std::optional<std::reference_wrapper<Color>> m_externalRef;
	std::function<void(const Color&)> m_onChange;
	std::function<void(const Color&, void*)> m_onChangeWithWidget;
};

// SeparatorWrapper -----------------------------------------------------------
class SeparatorWrapper : public ControlWrapper
{
public:
	SeparatorWrapper(
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif
};

// ProgressBarWrapper -----------------------------------------------------------
class ProgressBarWrapper : public ControlWrapper
{
public:
	ProgressBarWrapper(const float& value,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
	{
	}
	ProgressBarWrapper(float& value,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_ownedValue(value)
		, m_externalRef(value)
	{
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	float m_ownedValue;
	std::optional<std::reference_wrapper<float>> m_externalRef;
};

// ComboBoxWrapper -----------------------------------------------------------
template <ComboBoxValue T>
class ComboBoxWrapper : public ControlWrapper
{
public:
	ComboBoxWrapper(std::vector<std::string> choices,
		T& selected, const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {},
		std::function<void(const T&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_choices(std::move(choices))
		, m_ownedSelected(selected)
		, m_externalRef(selected)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
		buildItems();
	}
	ComboBoxWrapper(std::vector<std::string> choices,
		const T& selected, const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {},
		std::function<void(const T&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_choices(std::move(choices))
		, m_ownedSelected(selected)
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
		buildItems();
	}

#if defined(USE_WX) || defined(USE_QT)
	void realize(void* parentWindow) override;
#endif
#ifdef USE_IMGUI
	Size measureIntrinsic(const Constraints& c) override;
	void render(const Rect& frame) override;
#endif

private:
	// Builds the '\0'-separated item string and resolves the initial index
	// from the selection (used by the ImGui backend).
	void buildItems()
	{
		for (const auto& c : m_choices)
		{
			m_items += c;
			m_items += '\0';
		}

		if constexpr (std::is_same_v<T, int>)
		{
			m_currentItem = m_ownedSelected;
		}
		else
		{
			for (int i = 0; i < static_cast<int>(m_choices.size()); ++i)
			{
				if (m_choices[i] == m_ownedSelected)
				{
					m_currentItem = i;
					break;
				}
			}
		}
	}

	std::string m_items;
	std::vector<std::string> m_choices;
	int m_currentItem = 0;
	T m_ownedSelected{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
};

extern template class ComboBoxWrapper<std::string>;
extern template class ComboBoxWrapper<int>;
