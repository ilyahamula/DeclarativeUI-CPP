#pragma once

#include "ControlWrapper.hpp"
#include "frameworks_core/CoreTypes/BoundValue.hpp"

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
//  - A control's value arrives as a BoundValue<T>, taken by value: copying it
//    carries over the caller's binding when there is one and the snapshot
//    otherwise, so there is one constructor rather than a bound/unbound pair,
//    and no wrapper can end up referring to the widget that built it.
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
	TextCtrlWrapper(BoundValue<std::string> value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {},
		std::function<void(const std::string&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
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
	BoundValue<std::string> m_value;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
};

// PasswordInputWrapper -----------------------------------------------------------
class PasswordInputWrapper : public ControlWrapper
{
public:
	PasswordInputWrapper(BoundValue<std::string> value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {},
		std::function<void(const std::string&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
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
	BoundValue<std::string> m_value;
	std::function<void(const std::string&)> m_onChange;
	std::function<void(const std::string&, void*)> m_onChangeWithWidget;
};

// MultiLineTextCtrlWrapper -----------------------------------------------------------
class MultiLineTextCtrlWrapper : public ControlWrapper
{
public:
	MultiLineTextCtrlWrapper(BoundValue<std::string> value,
		const Position& pos, const Size& size, long style,
		std::function<void(const std::string&)> onChange = {},
		std::function<void(const std::string&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
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
	BoundValue<std::string> m_value;
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
	DatePickerWrapper(BoundValue<Date> value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Date&)> onChange = {},
		std::function<void(const Date&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
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
	BoundValue<Date> m_value;
	std::function<void(const Date&)> m_onChange;
	std::function<void(const Date&, void*)> m_onChangeWithWidget;
};

// TimePickerWrapper -----------------------------------------------------------
class TimePickerWrapper : public ControlWrapper
{
public:
	TimePickerWrapper(BoundValue<Time> value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Time&)> onChange = {},
		std::function<void(const Time&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
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
	BoundValue<Time> m_value;
	std::function<void(const Time&)> m_onChange;
	std::function<void(const Time&, void*)> m_onChangeWithWidget;
};

// SliderWrapper -----------------------------------------------------------
template <SliderValue T>
class SliderWrapper : public ControlWrapper
{
public:
	SliderWrapper(Range<T> range, BoundValue<T> value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {},
		std::function<void(T, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_range(range)
		, m_value(std::move(value))
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
	BoundValue<T> m_value;
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
	SpinBoxWrapper(Range<T> range, BoundValue<T> value,
		const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {},
		std::function<void(T, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_range(range)
		, m_value(std::move(value))
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
	BoundValue<T> m_value;
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
		BoundValue<T> value, const Position& pos, const Size& size, long style,
		std::function<void(T)> onChange = {},
		std::function<void(T, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_value(std::move(value))
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
		assignGroupIndex(&m_value.get());
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
	BoundValue<T> m_value;
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
		BoundValue<bool> checked,
		std::function<void(bool)> onChange = {},
		std::function<void(bool, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_value(std::move(checked))
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
	BoundValue<bool> m_value;
	std::function<void(bool)> m_onChange;
	std::function<void(bool, void*)> m_onChangeWithWidget;
};

// ToggleButtonWrapper -----------------------------------------------------------
class ToggleButtonWrapper : public ControlWrapper
{
public:
	ToggleButtonWrapper(const std::string& label,
		BoundValue<bool> toggled, const Position& pos, const Size& size, long style,
		std::function<void(bool)> onChange = {},
		std::function<void(bool, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_value(std::move(toggled))
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
	BoundValue<bool> m_value;
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
	ColorPickerWrapper(BoundValue<Color> value,
		const Position& pos, const Size& size, long style,
		std::function<void(const Color&)> onChange = {},
		std::function<void(const Color&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
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
	BoundValue<Color> m_value;
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
	ProgressBarWrapper(BoundValue<float> value,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
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
	BoundValue<float> m_value;
};

// ComboBoxWrapper -----------------------------------------------------------
template <ComboBoxValue T>
class ComboBoxWrapper : public ControlWrapper
{
public:
	ComboBoxWrapper(std::vector<std::string> choices,
		BoundValue<T> selected, const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {},
		std::function<void(const T&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_choices(std::move(choices))
		, m_value(std::move(selected))
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
			m_currentItem = m_value.get();
		}
		else
		{
			for (int i = 0; i < static_cast<int>(m_choices.size()); ++i)
			{
				if (m_choices[i] == m_value.get())
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
	BoundValue<T> m_value;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
};

extern template class ComboBoxWrapper<std::string>;
extern template class ComboBoxWrapper<int>;

// ListBoxWrapper -----------------------------------------------------------
// Single- or multi-select depending on T (see ListBoxValue). The selection is
// carried as item indices everywhere inside the wrapper -- indicesFor() and
// valueFor() are the only two places the bound type is decoded, so all three
// backends share one interpretation of it.
template <ListBoxValue T>
class ListBoxWrapper : public ControlWrapper
{
public:
	static constexpr bool kMultiSelect = MultiSelectListBoxValue<T>;

	ListBoxWrapper(std::vector<std::string> items,
		BoundValue<T> selected, int visibleRows, const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {},
		std::function<void(const T&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_items(std::move(items))
		, m_visibleRows(visibleRows)
		, m_value(std::move(selected))
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

	// Item indices the control should show for `value`. Out-of-range entries
	// are dropped rather than clamped: a stale index means "not in this list",
	// and silently selecting a neighbour would be worse than selecting nothing.
	//
	// Static, and taking the items explicitly, because the retained backends call
	// it from event handlers and idle syncs: those must not capture the wrapper
	// (see the note in wx/RefSync.hpp -- wrapper and window teardown order is not
	// fixed), only the item list they copy.
	static std::vector<int> indicesFor(const std::vector<std::string>& items, const T& value)
	{
		std::vector<int> indices;
		const int count = static_cast<int>(items.size());
		const auto addIndexOf = [&](const ListBoxItem<T>& item) {
			if constexpr (std::is_same_v<ListBoxItem<T>, int>)
			{
				if (item >= 0 && item < count)
					indices.push_back(item);
			}
			else
			{
				for (int i = 0; i < count; ++i)
				{
					if (items[i] == item)
					{
						indices.push_back(i);
						break;
					}
				}
			}
		};

		if constexpr (kMultiSelect)
		{
			for (const auto& item : value)
				addIndexOf(item);
		}
		else
		{
			addIndexOf(value);
		}
		return indices;
	}

	// The inverse: the bound value for a set of selected indices. A single-select
	// binding with nothing selected reports -1 / "" -- the same "no selection"
	// wxNOT_FOUND spelling the retained backends use.
	static T valueFor(const std::vector<std::string>& items, const std::vector<int>& indices)
	{
		const auto itemAt = [&](int i) -> ListBoxItem<T> {
			if constexpr (std::is_same_v<ListBoxItem<T>, int>)
				return i;
			else
				return (i >= 0 && i < static_cast<int>(items.size())) ? items[i] : std::string{};
		};

		if constexpr (kMultiSelect)
		{
			T value;
			value.reserve(indices.size());
			for (int i : indices)
				value.push_back(itemAt(i));
			return value;
		}
		else
		{
			return indices.empty() ? itemAt(-1) : itemAt(indices.front());
		}
	}

	// Commit a new selection: the value first, then the user callback, so a
	// handler reading the bound value sees the new one.
	void commit(const std::vector<int>& indices)
	{
		m_value.set(valueFor(m_items, indices));
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
	}

	const T& boundValue() const { return m_value.get(); }

private:
	std::vector<std::string> m_items;
	int m_visibleRows = 1;
	BoundValue<T> m_value;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
};

extern template class ListBoxWrapper<int>;
extern template class ListBoxWrapper<std::string>;
extern template class ListBoxWrapper<std::vector<int>>;
extern template class ListBoxWrapper<std::vector<std::string>>;
