#pragma once

#include "ControlWrapper.hpp"
#include "frameworks_core/CoreTypes/BoundValue.hpp"

#include <algorithm>
#include <cstdio>
#include <functional>
#include <numeric>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Every wrapper declares the same shape of per-backend override: realize() on
// the retained backends (wx/Qt), measureIntrinsic()/render() on ImGui (see the
// wrapper contract below). One macro call replaces that repeated block.
#if defined(USE_WX) || defined(USE_QT)
#define DECLARE_CONTROL_WRAPPER_OVERRIDES() \
	void realize(void* parentWindow) override
#elif defined(USE_IMGUI)
#define DECLARE_CONTROL_WRAPPER_OVERRIDES() \
	Size measureIntrinsic(const Constraints& c) override; \
	void render(const Rect& frame) override
#endif

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	std::string m_value;
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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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
	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

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

// TreeViewWrapper -----------------------------------------------------------
// Hierarchical, collapsible item list. The selection is carried as item PATHS
// ("Fruits/Apple") everywhere inside the wrapper -- pathsFor() and valueFor()
// are the only two places the bound type is decoded, so all three backends
// share one interpretation of it, exactly as ListBoxWrapper does with indices.
//
// Paths rather than indices because an index is meaningless in a tree, and the
// native handles that would otherwise identify an item (wxTreeItemId,
// QTreeWidgetItem*) are backend types that cannot reach the public API.
//
// Multi-select is a runtime flag, not a property of T: a tree is single-select
// by default on every binding, and TreeView::isMultiSelect() widens it.
template <TreeViewValue T>
class TreeViewWrapper : public ControlWrapper
{
public:
	// A label containing this would produce an ambiguous path. Callers author
	// their own labels, so that is documented rather than escaped -- escaping
	// would make the bound value awkward to compare against a literal, which
	// is the whole point of binding paths instead of handles.
	static constexpr char kPathSeparator = '/';

	static constexpr bool kVectorBinding = MultiSelectTreeViewValue<T>;

	TreeViewWrapper(std::vector<TreeItem> items,
		BoundValue<T> selected, int visibleRows, bool multiSelect,
		const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {},
		std::function<void(const T&, void*)> onChangeWithWidget = {})
		: ControlWrapper(pos, size, style)
		, m_items(std::move(items))
		, m_visibleRows(visibleRows)
		, m_multiSelect(multiSelect)
		, m_value(std::move(selected))
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

	static std::string joinPath(const std::string& parentPath, const std::string& label)
	{
		return parentPath.empty() ? label : parentPath + kPathSeparator + label;
	}

	// Depth-first walk in declaration order, handing `fn` each item, its full
	// path and its depth. Backends that need the parent handle to build their
	// native tree recurse themselves; this is for the passes that only need the
	// flat sequence, chiefly measurement.
	//
	// Static, and taking the items explicitly, for the same reason the decoders
	// below are: the retained backends call these from event handlers and idle
	// syncs, which must not capture the wrapper (see wx/RefSync.hpp -- wrapper
	// and window teardown order is not fixed), only the items they copy.
	template <typename Fn>
	static void forEachItem(const std::vector<TreeItem>& items, const Fn& fn,
		const std::string& parentPath = {}, int depth = 0)
	{
		for (const TreeItem& item : items)
		{
			const std::string path = joinPath(parentPath, item.label);
			fn(item, path, depth);
			forEachItem(item.children, fn, path, depth + 1);
		}
	}

	// Paths the control should show selected. Empty entries are dropped: "" is
	// how a single-path binding spells "nothing selected", and it matches no
	// item -- a path is only ever empty above the first label.
	static std::vector<std::string> pathsFor(const T& value)
	{
		std::vector<std::string> paths;
		if constexpr (kVectorBinding)
		{
			for (const std::string& path : value)
			{
				if (!path.empty())
					paths.push_back(path);
			}
		}
		else if (!value.empty())
		{
			paths.push_back(value);
		}
		return paths;
	}

	// The inverse. A single-path binding with nothing selected reports "" --
	// the same "no selection" spelling pathsFor() drops. A vector binding left
	// single-select simply never receives more than one path.
	static T valueFor(const std::vector<std::string>& paths)
	{
		if constexpr (kVectorBinding)
			return T(paths.begin(), paths.end());
		else
			return paths.empty() ? std::string {} : paths.front();
	}

	// Commit a new selection: the value first, then the user callback, so a
	// handler reading the bound value sees the new one.
	void commit(const std::vector<std::string>& paths)
	{
		m_value.set(valueFor(paths));
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
	}

	const T& boundValue() const { return m_value.get(); }

private:
	std::vector<TreeItem> m_items;
	int m_visibleRows = 1;
	bool m_multiSelect = false;
	BoundValue<T> m_value;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
};

extern template class TreeViewWrapper<std::string>;
extern template class TreeViewWrapper<std::vector<std::string>>;

// TableWrapper -----------------------------------------------------------
// Multi-column tabular display: a column list, a rectangular block of text
// cells, row selection, optional per-column sorting and optional per-column
// cell editing.
//
// Rows are identified by their ORIGINAL index -- their position in the TableRows
// the caller passed -- and never by their position on screen. Sorting reorders
// the display, so those two part company the moment a header is clicked, and a
// binding that tracked the display would shift underneath the caller for free.
// Every backend keeps the original index on the row it builds (wx in
// SetItemData, Qt in a UserRole, ImGui in its own permutation) and decodes it
// back through rowIndicesFor()/valueFor() -- the only two places the bound type
// is interpreted, so all three share one reading of it, exactly as
// ListBoxWrapper does with its indices.
//
// The ROWS are a BoundValue as well as the selection, which is what makes cell
// editing work: bound, an edit writes through to the caller's data and is what
// every backend redraws from; unbound, it lands in a snapshot this wrapper owns
// and only onCellChange observes it. That distinction is invisible on wx and Qt
// -- their native control retains the edited text either way -- but decides the
// behaviour on ImGui, where the whole tree is rebuilt every frame and an edit
// with nowhere caller-owned to live is gone by the next one.
template <TableValue T>
class TableWrapper : public ControlWrapper
{
public:
	static constexpr bool kMultiSelect = MultiSelectTableValue<T>;

	// The column a std::string binding reads its key from. First rather than
	// configurable: a key column is a property of how the caller laid the table
	// out, and every backend already treats column 0 as the row's identity.
	static constexpr int kKeyColumn = 0;

	TableWrapper(std::vector<TableColumn> columns,
		BoundValue<TableRows> rows,
		BoundValue<T> selected, int visibleRows,
		const Position& pos, const Size& size, long style,
		std::function<void(const T&)> onChange = {},
		std::function<void(const T&, void*)> onChangeWithWidget = {},
		std::function<void(int, int, const std::string&)> onCellChange = {})
		: ControlWrapper(pos, size, style)
		, m_columns(std::move(columns))
		, m_rows(std::move(rows))
		, m_visibleRows(visibleRows)
		, m_value(std::move(selected))
		, m_onChange(std::move(onChange))
		, m_onChangeWithWidget(std::move(onChangeWithWidget))
		, m_onCellChange(std::move(onCellChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

	// A cell's text, or "" for a ragged row that is short of this column. Rows
	// are not required to be the same length as the column list: a short row is
	// read as trailing empty cells rather than rejected, so a table built from
	// partial data still displays.
	static const std::string& cellText(const TableRows& rows, int row, int column)
	{
		static const std::string kEmpty;
		if (row < 0 || row >= static_cast<int>(rows.size()))
			return kEmpty;
		const TableRow& cells = rows[row];
		if (column < 0 || column >= static_cast<int>(cells.size()))
			return kEmpty;
		return cells[column];
	}

	// Original row indices the control should show selected. Out-of-range
	// indices and unmatched keys are dropped rather than clamped, for the reason
	// ListBoxWrapper drops them: a stale row reference means "not in this table",
	// and quietly selecting a neighbour would be worse than selecting nothing.
	//
	// Static, and taking the rows explicitly, because the retained backends call
	// it from event handlers and idle syncs: those must not capture the wrapper
	// (see the note in wx/RefSync.hpp -- wrapper and window teardown order is not
	// fixed), only the data they copy.
	static std::vector<int> rowIndicesFor(const TableRows& rows, const T& value)
	{
		std::vector<int> indices;
		const int count = static_cast<int>(rows.size());
		const auto addRow = [&](const TableKey<T>& key) {
			if constexpr (std::is_same_v<TableKey<T>, int>)
			{
				if (key >= 0 && key < count)
					indices.push_back(key);
			}
			else
			{
				// "" is how a key binding spells "nothing selected" -- it is what
				// valueFor() reports for an empty selection -- so it must not
				// match, or clearing the selection would land on the first row
				// with an empty first cell.
				if (key.empty())
					return;
				// First match wins: duplicate keys are the documented weakness
				// of a key binding, not something to resolve arbitrarily here.
				for (int i = 0; i < count; ++i)
				{
					if (cellText(rows, i, kKeyColumn) == key)
					{
						indices.push_back(i);
						break;
					}
				}
			}
		};

		if constexpr (kMultiSelect)
		{
			for (const auto& key : value)
				addRow(key);
		}
		else
		{
			addRow(value);
		}
		return indices;
	}

	// The inverse. A single-select binding with nothing selected reports -1 / ""
	// -- the same "no selection" spelling ListBoxWrapper uses, and the one
	// rowIndicesFor() drops on the way back in.
	static T valueFor(const TableRows& rows, const std::vector<int>& indices)
	{
		const auto keyAt = [&](int i) -> TableKey<T> {
			if constexpr (std::is_same_v<TableKey<T>, int>)
				return i;
			else
				return cellText(rows, i, kKeyColumn);
		};

		if constexpr (kMultiSelect)
		{
			T value;
			value.reserve(indices.size());
			for (int i : indices)
				value.push_back(keyAt(i));
			return value;
		}
		else
		{
			return indices.empty() ? keyAt(-1) : keyAt(indices.front());
		}
	}

	// Original row indices in the order `column` sorts them, ascending or not.
	// Only ImGui calls this -- wx and Qt sort natively -- but the comparison is
	// deliberately the same one they use: a lexicographic compare of the cell
	// TEXT. A table holds text, so "10" sorts before "9" on all three backends
	// rather than only on the two that happen to share a collation.
	//
	// stable_sort so that equal cells keep their declaration order, which is the
	// only tie-break a caller can predict.
	static std::vector<int> sortedOrder(const TableRows& rows, int column, bool ascending)
	{
		std::vector<int> order(rows.size());
		std::iota(order.begin(), order.end(), 0);
		if (column < 0)
			return order;

		std::stable_sort(order.begin(), order.end(), [&](int lhs, int rhs) {
			const std::string& a = cellText(rows, lhs, column);
			const std::string& b = cellText(rows, rhs, column);
			return ascending ? a < b : b < a;
		});
		return order;
	}

	// Column width policy, shared by all three backends: an explicit
	// TableColumn::width wins, otherwise the column is as wide as the widest of
	// its header and its cells. The policy belongs here; the metrics cannot --
	// `measureText` is the backend's own text measurement.
	template <typename MeasureText>
	static int columnWidth(const std::vector<TableColumn>& columns, const TableRows& rows,
		int column, const MeasureText& measureText)
	{
		if (columns[column].width > 0)
			return columns[column].width;

		int widest = measureText(columns[column].label);
		for (int row = 0; row < static_cast<int>(rows.size()); ++row)
			widest = std::max(widest, measureText(cellText(rows, row, column)));
		return widest;
	}

	// Write an edited cell back into caller-owned rows. Static and taking the
	// target explicitly so a retained backend's edit handler can call it holding
	// only the pointer from boundRows() -- never the wrapper.
	//
	// A short row is padded rather than skipped, matching cellText()'s reading
	// of ragged rows as trailing empty cells.
	static void applyCellEdit(TableRows* rows, int row, int column, const std::string& text)
	{
		if (!rows || row < 0 || row >= static_cast<int>(rows->size()) || column < 0)
			return;
		TableRow& cells = (*rows)[row];
		if (column >= static_cast<int>(cells.size()))
			cells.resize(column + 1);
		cells[column] = text;
	}

	// Commit a new selection: the value first, then the user callback, so a
	// handler reading the bound value sees the new one.
	void commit(const std::vector<int>& indices)
	{
		m_value.set(valueFor(m_rows.get(), indices));
		if (m_onChange)
			m_onChange(m_value.get());
		else if (m_onChangeWithWidget)
			m_onChangeWithWidget(m_value.get(), m_nativeWidget);
	}

	// Commit a cell edit, in the same order and for the same reason: the data
	// first, then the callback. `row` is an original index.
	void commitCell(int row, int column, const std::string& text)
	{
		applyCellEdit(&m_rows.get(), row, column, text);
		if (m_onCellChange)
			m_onCellChange(row, column, text);
	}

	const T& boundValue() const { return m_value.get(); }

	// Non-null only while the rows are bound: the caller-owned table an edit may
	// write through to. A snapshot reports nullptr -- see the note above on what
	// that costs on ImGui.
	TableRows* boundRows() { return m_rows.isBound() ? &m_rows.get() : nullptr; }

private:
	std::vector<TableColumn> m_columns;
	BoundValue<TableRows> m_rows;
	int m_visibleRows = 1;
	BoundValue<T> m_value;
	std::function<void(const T&)> m_onChange;
	std::function<void(const T&, void*)> m_onChangeWithWidget;
	std::function<void(int, int, const std::string&)> m_onCellChange;
};

extern template class TableWrapper<int>;
extern template class TableWrapper<std::string>;
extern template class TableWrapper<std::vector<int>>;
extern template class TableWrapper<std::vector<std::string>>;
