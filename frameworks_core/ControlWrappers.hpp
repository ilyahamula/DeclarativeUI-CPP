#pragma once

#include "ControlWrapper.hpp"
#include "frameworks_core/CoreTypes/BoundValue.hpp"
#include "frameworks_core/CoreTypes/Calendar.hpp"
#include "frameworks_core/CoreTypes/DialogKeys.hpp"
#include "frameworks_core/CoreTypes/EventCallback.hpp"
#include "frameworks_core/CoreTypes/Spinner.hpp"
#include "frameworks_core/CoreTypes/StatusField.hpp"
#include "frameworks_core/CoreTypes/TextField.hpp"
#include "frameworks_core/CoreTypes/ToolItem.hpp"
#include "frameworks_core/CoreTypes/ExpanderState.hpp"
#include "frameworks_core/CoreTypes/FileFilter.hpp"
#include "frameworks_core/CoreTypes/SplitterState.hpp"
#include "frameworks_core/CoreTypes/RichTextRuns.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Every wrapper declares the same shape of per-backend override: realize() on
// the retained backends (wx/Qt), measureIntrinsic()/render() on ImGui (see the
// wrapper contract below). One macro call replaces that repeated block, and
// the few variations below get one macro each, so no wrapper spells out a
// backend #if of its own. A macro with nothing to declare on a backend expands
// to static_assert(true, ""), so every call site is written `MACRO();`.
//
//   DECLARE_CONTROL_WRAPPER_OVERRIDES()     the usual shape (see above).
//   DECLARE_SELF_MEASURED_OVERRIDES(cond)   wx/Qt: the wrapper answers its own
//       measure whenever `cond` holds -- a bound display frozen at its first
//       content, or a height that follows the offered width -- instead of the
//       native best size (ControlWrapper::measuresItself). ImGui always
//       measures through the wrapper, so there it declares nothing.
//   DECLARE_MEASURES_ITSELF(cond)           the same, for a wrapper whose
//       measureIntrinsic() is one inline body shared by all three backends.
//   DECLARE_PLACED_OVERRIDE()               wx/Qt: placed(frame), for content
//       composed against the frame the engine assigned (ControlWrapper::placed).
//   DECLARE_DRAW_OVERRIDES()                realize() / render() only, for a
//       wrapper whose measure is shared inline.
//   DECLARE_WINDOWLESS_OVERRIDES()          render() on ImGui and nothing on
//       wx/Qt, for a leaf that creates no native window (Spacer).
#if defined(USE_WX) || defined(USE_QT)
#define DECLARE_CONTROL_WRAPPER_OVERRIDES() \
	void realize(void* parentWindow) override
#define DECLARE_MEASURES_ITSELF(cond) \
	bool measuresItself() const override { return (cond); }
#define DECLARE_SELF_MEASURED_OVERRIDES(cond) \
	Size measureIntrinsic(const Constraints& c) override; \
	DECLARE_MEASURES_ITSELF(cond)
#define DECLARE_PLACED_OVERRIDE() \
	void placed(const Rect& frame) override
#define DECLARE_DRAW_OVERRIDES() \
	void realize(void* parentWindow) override
#define DECLARE_WINDOWLESS_OVERRIDES() \
	static_assert(true, "")
#elif defined(USE_IMGUI)
#define DECLARE_CONTROL_WRAPPER_OVERRIDES() \
	Size measureIntrinsic(const Constraints& c) override; \
	void render(const Rect& frame) override
#define DECLARE_MEASURES_ITSELF(cond) \
	static_assert(true, "")
#define DECLARE_SELF_MEASURED_OVERRIDES(cond) \
	static_assert(true, "")
#define DECLARE_PLACED_OVERRIDE() \
	static_assert(true, "")
#define DECLARE_DRAW_OVERRIDES() \
	void render(const Rect& frame) override
#define DECLARE_WINDOWLESS_OVERRIDES() \
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
		EventCallback<> onClick = {}, unsigned dialogKeys = kNoDialogKey,
		std::string iconPath = {}, Size iconSize = { 16, 16 })
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_onClick(std::move(onClick))
		, m_dialogKeys(dialogKeys)
		, m_iconPath(std::move(iconPath))
		, m_iconSize(iconSize)
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	std::string m_label;
	EventCallback<> m_onClick;
	unsigned m_dialogKeys; // DialogKeyRole bits
	// An image left of the label (Button::withIcon). Empty, or a path that
	// fails to load, is a plain text button -- logged, never fatal, as a
	// ToolItem's icon is.
	std::string m_iconPath;
	Size m_iconSize;
};

// TextCtrlWrapper -----------------------------------------------------------
// The placeholder is a plain string, not a BoundValue: it is a fixed label for
// the empty field rather than a value anything writes, so there is nothing for
// a ref sync to poll. It is deliberately NOT measured on any backend -- a hint
// is usually longer than the text it stands in for, and measuring it would let
// wording resize an auto-fit dialog, which is the trap the editable fields'
// content-independent floors already avoid.
class TextCtrlWrapper : public ControlWrapper
{
public:
	TextCtrlWrapper(BoundValue<std::string> value, std::string placeholder,
		const Position& pos, const Size& size, long style,
		EventCallback<const std::string&> onChange = {},
		EventCallback<const std::string&> onEnter = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_placeholder(std::move(placeholder))
		, m_onChange(std::move(onChange))
		, m_onEnter(std::move(onEnter))
	{
	}

	// Focus and validity (TextField.hpp), set by the widget after construction.
	void setFieldOptions(TextFieldOptions options) { m_field = std::move(options); }

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	TextFieldOptions m_field;
	BoundValue<std::string> m_value;
	std::string m_placeholder;
	EventCallback<const std::string&> m_onChange;
	// Enter in the field, after the text is committed. The window's default
	// button (if any) is pressed right after it, as a native dialog would.
	EventCallback<const std::string&> m_onEnter;
};

// PasswordInputWrapper -----------------------------------------------------------
class PasswordInputWrapper : public ControlWrapper
{
public:
	PasswordInputWrapper(BoundValue<std::string> value, std::string placeholder,
		const Position& pos, const Size& size, long style,
		EventCallback<const std::string&> onChange = {},
		EventCallback<const std::string&> onEnter = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_placeholder(std::move(placeholder))
		, m_onChange(std::move(onChange))
		, m_onEnter(std::move(onEnter))
	{
	}

	// Focus and validity (TextField.hpp), set by the widget after construction.
	void setFieldOptions(TextFieldOptions options) { m_field = std::move(options); }

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	TextFieldOptions m_field;
	BoundValue<std::string> m_value;
	std::string m_placeholder;
	EventCallback<const std::string&> m_onChange;
	// Enter in the field, after the text is committed. The window's default
	// button (if any) is pressed right after it, as a native dialog would.
	EventCallback<const std::string&> m_onEnter;
};

// SearchFieldWrapper -----------------------------------------------------------
// A single-line field for a search query: a magnifier on the left, a clear
// button on the right while there is text, and onSearch on Enter. Native on wx
// (wxSearchCtrl), a QLineEdit with Qt's own clear button on Qt, composed on
// ImGui. Its Enter belongs to it: onSearch runs and the window's default button
// is NOT pressed -- a search box inside a form searches, it does not submit.
// Clearing is an ordinary edit (onChange with ""), never an onSearch.
class SearchFieldWrapper : public ControlWrapper
{
public:
	SearchFieldWrapper(BoundValue<std::string> value, std::string placeholder,
		const Position& pos, const Size& size, long style,
		EventCallback<const std::string&> onChange = {},
		EventCallback<const std::string&> onSearch = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_placeholder(std::move(placeholder))
		, m_onChange(std::move(onChange))
		, m_onSearch(std::move(onSearch))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	BoundValue<std::string> m_value;
	std::string m_placeholder;
	EventCallback<const std::string&> m_onChange;
	EventCallback<const std::string&> m_onSearch;
};

// MultiLineTextCtrlWrapper -----------------------------------------------------------
class MultiLineTextCtrlWrapper : public ControlWrapper
{
public:
	MultiLineTextCtrlWrapper(BoundValue<std::string> value,
		const Position& pos, const Size& size, long style,
		EventCallback<const std::string&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_onChange(std::move(onChange))
	{
	}

	// Focus and validity (TextField.hpp), set by the widget after construction.
	void setFieldOptions(TextFieldOptions options) { m_field = std::move(options); }

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	TextFieldOptions m_field;
	BoundValue<std::string> m_value;
	EventCallback<const std::string&> m_onChange;
};

// ReadonlyTextCtrlWrapper -----------------------------------------------------------
class ReadonlyTextCtrlWrapper : public ControlWrapper
{
public:
	ReadonlyTextCtrlWrapper(BoundValue<std::string> value,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	BoundValue<std::string> m_value;
};

// ClickableTextWrapper -----------------------------------------------------------
class ClickableTextWrapper : public ControlWrapper
{
public:
	ClickableTextWrapper(const std::string& text,
		const Position& pos, const Size& size, long style,
		EventCallback<> onClick = {})
		: ControlWrapper(pos, size, style)
		, m_text(text)
		, m_onClick(std::move(onClick))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	std::string m_text;
	EventCallback<> m_onClick;
};

// LinkTextWrapper -----------------------------------------------------------
class LinkTextWrapper : public ControlWrapper
{
public:
	LinkTextWrapper(const std::string& text,
		const Position& pos, const Size& size, long style,
		EventCallback<> onClick = {})
		: ControlWrapper(pos, size, style)
		, m_text(text)
		, m_onClick(std::move(onClick))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	std::string m_text;
	EventCallback<> m_onClick;
};

// StaticTextWrapper -----------------------------------------------------------
// The alignment is a plain TextAlign, not a BoundValue, for the same reason a
// placeholder is a plain string: it is decided when the tree is described and
// nothing writes it afterwards, so there is nothing for a ref sync to poll.
//
// A BOUND label measures its first text only (see StaticText): on wx/Qt the
// size is captured at realize() and answered through measuresItself(), since
// the native best size would follow every SetLabel; on ImGui it is parked
// under a measure-phase key the first time the label is measured.
class StaticTextWrapper : public ControlWrapper
{
public:
	StaticTextWrapper(BoundValue<std::string> text, TextAlign align,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_text(std::move(text))
		, m_align(align)
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	DECLARE_SELF_MEASURED_OVERRIDES(m_text.isBound());

private:
	BoundValue<std::string> m_text;
	TextAlign m_align = TextAlign::Left;
	Size m_initialSize { 0, 0 }; // wx/Qt, bound only: the size of the first text
};

// RichTextWrapper -----------------------------------------------------------
// The markup is parsed here, once, into the runs every backend draws from. No
// backend has a native control behind it: each draws the fragments
// layoutRichText() places, measured with its own fonts, so the wrapping rule is
// the same everywhere. That also makes the height depend on the width on offer,
// which a retained backend's native best size cannot express -- hence
// measuresItself() and a measureIntrinsic() on wx and Qt too.
//
// The wrap width is the explicit withSize() width when there is one, else the
// constraint's: measureContent() replaces the measured width by the explicit
// one afterwards, and the height has to be the height AT that width.
class RichTextWrapper : public ControlWrapper
{
public:
	RichTextWrapper(std::string_view markup,
		const Position& pos, const Size& size, long style,
		EventCallback<const std::string&> onLink = {})
		: ControlWrapper(pos, size, style)
		, m_runs(parseMarkup(markup))
		, m_onLink(std::move(onLink))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	DECLARE_SELF_MEASURED_OVERRIDES(true);

private:
	int wrapWidth(const Constraints& c) const
	{
		return m_size.width > 0 ? m_size.width : c.maxWidth;
	}

	std::vector<TextRun> m_runs;
	EventCallback<const std::string&> m_onLink;
};

// DatePickerWrapper -----------------------------------------------------------
class DatePickerWrapper : public ControlWrapper
{
public:
	DatePickerWrapper(BoundValue<Date> value,
		const Position& pos, const Size& size, long style,
		EventCallback<const Date&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	BoundValue<Date> m_value;
	EventCallback<const Date&> m_onChange;
};

// TimePickerWrapper -----------------------------------------------------------
class TimePickerWrapper : public ControlWrapper
{
public:
	TimePickerWrapper(BoundValue<Time> value,
		const Position& pos, const Size& size, long style,
		EventCallback<const Time&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	BoundValue<Time> m_value;
	EventCallback<const Time&> m_onChange;
};

// SliderWrapper -----------------------------------------------------------
template <SliderValue T>
class SliderWrapper : public ControlWrapper
{
public:
	SliderWrapper(Range<T> range, BoundValue<T> value,
		const Position& pos, const Size& size, long style,
		EventCallback<T> onChange = {},
		Orientation orient = Orientation::Horizontal, T tickStep = T {})
		: ControlWrapper(pos, size, style)
		, m_range(range)
		, m_value(std::move(value))
		, m_onChange(std::move(onChange))
		, m_orient(orient)
		, m_tickStep(tickStep)
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

	// Where a tick sits along the track, 0 at the minimum end and 1 at the
	// maximum, for every multiple of the tick step from the minimum. Empty
	// when the slider has no ticks or the step would draw an unreadable comb.
	// Shared so the ImGui drawing and the tests read one rule.
	static std::vector<float> tickFractions(const Range<T>& range, T tickStep)
	{
		std::vector<float> ticks;
		const double span = static_cast<double>(range.max) - static_cast<double>(range.min);
		if (tickStep <= T {} || span <= 0.0)
			return ticks;
		const double count = span / static_cast<double>(tickStep);
		if (count > 200.0)
			return ticks;
		for (int i = 0; i <= static_cast<int>(count + 1e-9); ++i)
			ticks.push_back(static_cast<float>(i * static_cast<double>(tickStep) / span));
		return ticks;
	}

private:
	Range<T> m_range;
	BoundValue<T> m_value;
	EventCallback<T> m_onChange;
	// Vertical sliders have their MINIMUM AT THE BOTTOM on all three backends
	// (wx's default is the top; it is flipped with wxSL_INVERSE).
	Orientation m_orient;
	T m_tickStep; // value units between tick marks; 0 = none
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
		EventCallback<T> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_range(range)
		, m_value(std::move(value))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	Range<T> m_range;
	BoundValue<T> m_value;
	EventCallback<T> m_onChange;
};

extern template class SpinBoxWrapper<int>;
extern template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------
// One radio. An int radio is checked while the bound int equals `option`, and
// picking it writes `option`; the radios sharing that int ARE the group. That
// is the whole grouping rule, on every backend, and it holds no state of its
// own -- a group survives any rebuild because there is nothing to rebuild.
//
// No native radio group is used. Native grouping keys on creation order and on
// the parent window (a wxRB_GROUP run, Qt's auto-exclusive siblings or a
// QButtonGroup), neither of which is a property of the declarative tree: every
// leaf is parented flat to its dialog or page, so two groups in one box would
// merge natively. The retained backends therefore create each radio on its own
// (wxRB_SINGLE / setAutoExclusive(false)) and let the bound int, mirrored by
// the ordinary RefSync poll, uncheck the others. A bool radio ignores `option`.
template <RadioButtonValue T>
class RadioButtonWrapper : public ControlWrapper
{
public:
	RadioButtonWrapper(const std::string& label,
		BoundValue<T> value, int option, const Position& pos, const Size& size, long style,
		EventCallback<T> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_value(std::move(value))
		, m_option(option)
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

	// Whether a radio standing for `option` shows checked while the bound value
	// is `value`. Static and data-only, so an event handler can call it holding
	// nothing but the caller's variable -- never the wrapper.
	static bool isChecked(const T& value, int option)
	{
		if constexpr (std::is_same_v<T, bool>)
			return value;
		else
			return value == option;
	}

	// What picking a radio standing for `option` writes.
	static T picked(int option)
	{
		if constexpr (std::is_same_v<T, bool>)
			return true;
		else
			return option;
	}

private:
	std::string m_label;
	BoundValue<T> m_value;
	int m_option = 0;
	EventCallback<T> m_onChange;
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
		EventCallback<bool> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_value(std::move(checked))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	std::string m_label;
	BoundValue<bool> m_value;
	EventCallback<bool> m_onChange;
};

// ToggleButtonWrapper -----------------------------------------------------------
class ToggleButtonWrapper : public ControlWrapper
{
public:
	ToggleButtonWrapper(const std::string& label,
		BoundValue<bool> toggled, const Position& pos, const Size& size, long style,
		EventCallback<bool> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_value(std::move(toggled))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	std::string m_label;
	BoundValue<bool> m_value;
	EventCallback<bool> m_onChange;
};

// ImageWrapper -----------------------------------------------------------
class ImageWrapper : public ControlWrapper
{
public:
	ImageWrapper(const std::string& filePath, ScaleMode scaleMode,
		const Position& pos, const Size& size, long style,
		EventCallback<> onClick = {},
		EventCallback<> onHover = {})
		: ControlWrapper(pos, size, style)
		, m_filePath(filePath)
		, m_scaleMode(scaleMode)
		, m_displayWidth(size.width)
		, m_displayHeight(size.height)
		, m_onClick(std::move(onClick))
		, m_onHover(std::move(onHover))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	// The one wrapper that needs the frame AFTER the engine has computed it:
	// every mode but Stretch decides what the pixels do from the frame's shape,
	// and neither a wxStaticBitmap nor a QLabel can work that out for itself.
	DECLARE_PLACED_OVERRIDE();

private:
	std::string m_filePath;
	// What the pixels do inside the engine's frame. The frame itself is not
	// known until the control is placed, which is why every backend applies
	// this later than realize()/measure -- see the wrappers.
	ScaleMode m_scaleMode = ScaleMode::Stretch;
	void* m_textureId = nullptr; // ImTextureID (void*) holding the GL texture handle
	int m_imgWidth = 0;
	int m_imgHeight = 0;
	int m_displayWidth = -1;
	int m_displayHeight = -1;
	EventCallback<> m_onClick;
	EventCallback<> m_onHover;
};

// ToolBarWrapper -----------------------------------------------------------
// A row of command buttons. Unlike every other leaf here this one is a
// CONTAINER natively (a wxToolBar / QToolBar holding its own tools), but it is
// a LEAF to the engine: the tools are chrome the native control lays out
// itself, not nodes, so the engine sizes one rectangle and the backend fills
// it. That is why there is no NodeKind for it.
class ToolBarWrapper : public ControlWrapper
{
public:
	ToolBarWrapper(std::vector<ToolItem> tools, Size iconSize, bool labelsForced,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_tools(std::move(tools))
		, m_iconSize(iconSize)
		, m_labelsForced(labelsForced)
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	std::vector<ToolItem> m_tools;
	Size m_iconSize { 16, 16 };
	// Labels are shown for a tool that has no icon whatever this says; it
	// forces them on for the icon'd ones too.
	bool m_labelsForced = false;
};

// StatusBarWrapper -----------------------------------------------------------
// The row of read-only text an application keeps at its bottom edge. Like
// ToolBarWrapper this is a native CONTAINER but an engine LEAF: the native
// control owns its own panes, so the engine sizes one rectangle and the backend
// divides it.
class StatusBarWrapper : public ControlWrapper
{
public:
	StatusBarWrapper(StatusFields fields,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_fields(std::move(fields))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	// Width from statusBarContentWidth() on all three, height from the native
	// bar -- never the native best width, which differs per toolkit.
	DECLARE_SELF_MEASURED_OVERRIDES(true);

private:
	StatusFields m_fields;
};

// ColorPickerWrapper -----------------------------------------------------------
class ColorPickerWrapper : public ControlWrapper
{
public:
	ColorPickerWrapper(BoundValue<Color> value,
		const Position& pos, const Size& size, long style,
		EventCallback<const Color&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	BoundValue<Color> m_value;
	EventCallback<const Color&> m_onChange;
};

// FilePickerWrapper -----------------------------------------------------------
// A path field with a Browse button. ONE leaf to the engine on all three
// backends, though none of them builds it the same way: wx has a native picker
// control, Qt gets a composite QWidget laid out by hand, and ImGui draws a
// field, a button and -- since there is no OS dialog to open -- a browser popup
// of its own (imgui/FileBrowserPopup.hpp).
//
// Both ways of setting the path write through and fire onChange: picking one in
// the dialog, and typing one into the field (R11.4). That is why the wx picker
// asks for its text control and why Qt's composite carries a real QLineEdit
// rather than a read-only display.
class FilePickerWrapper : public ControlWrapper
{
public:
	FilePickerWrapper(BoundValue<std::string> value,
		FileMode mode, std::vector<FileFilter> filters, std::string dialogTitle,
		const Position& pos, const Size& size, long style,
		EventCallback<const std::string&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_mode(mode)
		, m_filters(std::move(filters))
		, m_dialogTitle(std::move(dialogTitle))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	BoundValue<std::string> m_value;
	FileMode m_mode = FileMode::Open;
	std::vector<FileFilter> m_filters;
	std::string m_dialogTitle;
	EventCallback<const std::string&> m_onChange;
};

// SpacerWrapper -----------------------------------------------------------
// The one windowless leaf: it creates nothing on any backend, so it declares no
// realize() (the base's no-op is exactly right, and the retained backends fall
// back to measureContent() while the native handle stays null) and it overrides
// measureIntrinsic() for every backend rather than through
// DECLARE_CONTROL_WRAPPER_OVERRIDES, whose ImGui half would leave wx and Qt
// measuring the base's {0, 0}. Its extent is pure data, so one body serves all
// three.
class SpacerWrapper : public ControlWrapper
{
public:
	SpacerWrapper(const Size& fixedSize,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_fixedSize(fixedSize)
	{
	}

	// {0, 0} for a flexible spacer: its extent comes from the Proportion the
	// widget defaults to, never from content.
	Size measureIntrinsic(const Constraints&) override
	{
		return m_fixedSize;
	}

	DECLARE_WINDOWLESS_OVERRIDES();

private:
	Size m_fixedSize;
};

// SeparatorWrapper -----------------------------------------------------------
// A hairline, and the SAME hairline on every backend: one pixel on its own axis
// and nothing on the other, measured here rather than asked of the native line
// -- wxStaticLine reports 2 px and a sunken QFrame 3, so a native measure would
// give one tree three different frames. The native line is simply drawn into
// the 1 px frame. It spans its parent once the caller adds Expand().
class SeparatorWrapper : public ControlWrapper
{
public:
	SeparatorWrapper(Orientation orient,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_orient(orient)
	{
	}

	DECLARE_DRAW_OVERRIDES();
	DECLARE_MEASURES_ITSELF(true);

	Size measureIntrinsic(const Constraints&) override
	{
		return m_orient == Orientation::Vertical ? Size { 1, 0 } : Size { 0, 1 };
	}

private:
	Orientation m_orient;
};

// SplitterSashWrapper -----------------------------------------------------------
// The draggable divider between a Splitter's two panes, and the one wrapper a
// caller never declares: Splitter::buildNode() creates it and hands it a pointer
// to the SplitterState living in its own node, which outlives every wrapper in
// the tree.
//
// There is no native splitter behind it on any backend -- wxSplitterWindow and
// QSplitter own their children's geometry and would fight the engine, which is
// the very thing the layout engine removed. What is left is a 6 px leaf that
// knows how to be dragged: it writes the first pane's new extent into the state
// (clamped against the bounds the arrange pass published there) and the engine
// re-divides the area on the next pass.
class SplitterSashWrapper : public ControlWrapper
{
public:
	SplitterSashWrapper(SplitterState* state,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_state(state)
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	SplitterState* m_state;

	// Where this sash's position is parked between ImGui frames. Taken during
	// measureIntrinsic() and reused by render(), which runs in a different
	// ImGui scope and could not derive the same key -- see the ImGui half of
	// ControlWrappers.cpp. Unused on the retained backends, where the wrapper
	// lives as long as the native window does.
	std::uint64_t m_snapshotKey = 0;
};

// ExpanderHeaderWrapper -----------------------------------------------------------
// The clickable title row of an Expander, and -- exactly like the Splitter's
// sash -- a leaf the caller never declares: Expander::buildNode() creates it and
// hands it a pointer to the ExpanderState living in its own node, which outlives
// every wrapper in the tree.
//
// Clicking it flips that state; the engine reads the state on its next measure
// pass and the section grows or shrinks with the dialog around it. The three
// backends draw the row natively where they can (wxCollapsibleHeaderCtrl, a
// checkable QToolButton) and by hand where a native row would not stay inside
// the engine's rectangle (ImGui's CollapsingHeader always spans the whole
// window, so the header is drawn on the draw list instead).
class ExpanderHeaderWrapper : public ControlWrapper
{
public:
	ExpanderHeaderWrapper(const std::string& label, ExpanderState* state,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_label(label)
		, m_state(state)
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	std::string m_label;
	ExpanderState* m_state;

	// Where this header's open state is parked between ImGui frames. Taken
	// during measureIntrinsic() and reused by render(), which runs in a
	// different ImGui scope and could not derive the same key -- see the ImGui
	// half of ControlWrappers.cpp. Unused on the retained backends, where the
	// wrapper lives as long as the native window does.
	std::uint64_t m_snapshotKey = 0;
};

// ProgressBarWrapper -----------------------------------------------------------
class ProgressBarWrapper : public ControlWrapper
{
public:
	ProgressBarWrapper(BoundValue<float> value, bool indeterminate,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_indeterminate(indeterminate)
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	BoundValue<float> m_value;
	// Busy mode: the value is ignored and the bar animates instead. Nothing polls
	// it, so it is a plain bool -- see ProgressBar::Indeterminate().
	bool m_indeterminate = false;
};

// Item lists -----------------------------------------------------------------
// The choices of a ComboBox, ListBox or CheckListBox: a snapshot, or the
// caller's vector BOUND, in which case the native control is repopulated when
// the vector changes (see the backends' syncItems).
using ItemList = std::vector<std::string>;

// The item list an event handler or RefSync poll reads: the caller's vector
// while bound, a shared copy of the snapshot otherwise. Holds no wrapper --
// handlers outlive it (wx/RefSync.hpp) -- and copies cheaply.
class ItemsView
{
public:
	explicit ItemsView(const BoundValue<ItemList>& items)
		: m_bound(items.boundValue())
		, m_snapshot(m_bound != nullptr ? nullptr : std::make_shared<const ItemList>(items.get()))
	{
	}

	const ItemList& operator()() const { return m_bound != nullptr ? *m_bound : *m_snapshot; }

	// Non-null only while bound: what a repopulating RefSync watches.
	const ItemList* bound() const { return m_bound; }

private:
	const ItemList* m_bound;
	std::shared_ptr<const ItemList> m_snapshot;
};

// ComboBoxWrapper -----------------------------------------------------------
template <ComboBoxValue T>
class ComboBoxWrapper : public ControlWrapper
{
public:
	ComboBoxWrapper(BoundValue<ItemList> choices,
		BoundValue<T> selected, const Position& pos, const Size& size, long style,
		EventCallback<const T&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_choices(std::move(choices))
		, m_value(std::move(selected))
		, m_onChange(std::move(onChange))
	{
		buildItems();
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	// Bound choices: answered with the size captured at realize() -- the native
	// best size would follow every repopulation (see ItemList).
	DECLARE_SELF_MEASURED_OVERRIDES(m_choices.isBound());

private:
	// Builds the '\0'-separated item string and resolves the initial index
	// from the selection (used by the ImGui backend).
	void buildItems()
	{
		for (const auto& c : m_choices.get())
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
			const ItemList& choices = m_choices.get();
			for (int i = 0; i < static_cast<int>(choices.size()); ++i)
			{
				if (choices[i] == m_value.get())
				{
					m_currentItem = i;
					break;
				}
			}
		}
	}

	std::string m_items;
	BoundValue<ItemList> m_choices;
	int m_currentItem = 0;
	BoundValue<T> m_value;
	EventCallback<const T&> m_onChange;
	Size m_initialSize { 0, 0 }; // wx/Qt, bound items only: the first list's size
};

extern template class ComboBoxWrapper<std::string>;
extern template class ComboBoxWrapper<int>;

// EditableComboWrapper -----------------------------------------------------------
// A text field with a drop-down of suggestions: the value is the TEXT, which
// may be any string -- picking an item writes that item's text, typing writes
// what was typed. The read-only choice is ComboBox; this is the other one.
//
// Native on wx (wxComboBox without wxCB_READONLY) and Qt (an editable
// QComboBox, NoInsert so Enter never adds the typed text to the list),
// composed on ImGui (a text field, an arrow button, a popup list). Enter is
// not the field's: it reaches the window's default button, as in a TextCtrl
// without onEnter. Bound items repopulate and keep the text; the control keeps
// the size of its first items, as a bound ComboBox does.
class EditableComboWrapper : public ControlWrapper
{
public:
	EditableComboWrapper(BoundValue<ItemList> items, BoundValue<std::string> text, std::string placeholder,
		const Position& pos, const Size& size, long style,
		EventCallback<const std::string&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_items(std::move(items))
		, m_value(std::move(text))
		, m_placeholder(std::move(placeholder))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	DECLARE_SELF_MEASURED_OVERRIDES(m_items.isBound());

private:
	BoundValue<ItemList> m_items;
	BoundValue<std::string> m_value;
	std::string m_placeholder;
	EventCallback<const std::string&> m_onChange;
	Size m_initialSize { 0, 0 }; // wx/Qt, bound items only: the first list's size
};

// VirtualListWrapper -----------------------------------------------------------
// A list that holds NO items: a row count and a function producing a row's
// text on demand, so a million rows cost what the visible ones do. wx: a
// wxLC_VIRTUAL report wxListCtrl asking OnGetItemText; Qt: a QListView over a
// model that asks the function in data(), uniform row heights; ImGui: a list
// box walked through ImGuiListClipper. The count is bound (it grows and the
// list follows); `revision` is an optional bound counter the caller bumps when
// rows CHANGED without the count changing. The selection is one row index, -1
// for none.
//
// Width cannot come from the content -- measuring every row is exactly what a
// virtual list exists to avoid -- so it measures kDefaultVirtualListWidth wide
// (withSize changes it) and visibleRows tall on every backend.
inline constexpr int kDefaultVirtualListWidth = 200;

class VirtualListWrapper : public ControlWrapper
{
public:
	VirtualListWrapper(BoundValue<int> count, std::function<std::string(int)> rowText,
		BoundValue<int> selected, BoundValue<int> revision, int visibleRows,
		const Position& pos, const Size& size, long style,
		EventCallback<int> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_count(std::move(count))
		, m_rowText(std::move(rowText))
		, m_value(std::move(selected))
		, m_revision(std::move(revision))
		, m_visibleRows(visibleRows)
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	DECLARE_SELF_MEASURED_OVERRIDES(true);

private:
	BoundValue<int> m_count;
	std::function<std::string(int)> m_rowText;
	BoundValue<int> m_value;
	BoundValue<int> m_revision;
	int m_visibleRows;
	EventCallback<int> m_onChange;
	Size m_initialSize { 0, 0 }; // wx/Qt: measured once at realize()
};

// CalendarWrapper -----------------------------------------------------------
// A month view; the value is the selected Date. Native on wx (wxCalendarCtrl:
// GTK and MSW native, the generic control on macOS) and Qt (QCalendarWidget,
// week numbers off to match), drawn on ImGui from CoreTypes/Calendar.hpp. All
// three are told the first day of the week -- except wxGTK, whose native
// calendar follows the system locale and has no setting for it. Each measures
// at its own natural size; the native calendars differ a lot, so a layout that
// must match on every backend pins one with withSize().
class CalendarWrapper : public ControlWrapper
{
public:
	CalendarWrapper(BoundValue<Date> value, FirstDayOfWeek firstDay,
		const Position& pos, const Size& size, long style,
		EventCallback<const Date&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_value(std::move(value))
		, m_firstDay(firstDay)
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();

private:
	BoundValue<Date> m_value;
	FirstDayOfWeek m_firstDay;
	EventCallback<const Date&> m_onChange;
};

// SpinnerWrapper -----------------------------------------------------------
// A busy indicator: no value, only "running or not". The default look is each
// backend's own -- wxActivityIndicator on wx, a turning arc on Qt and ImGui --
// and with an image (withImage) the IMAGE turns, drawn by the framework on all
// three: an owner-drawn panel on wx (wxGraphicsContext rotates the bitmap),
// a painted QWidget on Qt, a rotated texture quad on ImGui. A path that fails
// to load is the default spinner (logged).
//
// Stopped, the default spinner draws nothing and an image rests unrotated; the
// space is kept either way (isHidden removes it). It measures
// kDefaultSpinnerSize square itself on every backend, so withSize() is the one
// thing that changes it.
class SpinnerWrapper : public ControlWrapper
{
public:
	SpinnerWrapper(std::string imagePath, BoundValue<bool> running,
		const Position& pos, const Size& size, long style)
		: ControlWrapper(pos, size, style)
		, m_imagePath(std::move(imagePath))
		, m_running(std::move(running))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	DECLARE_SELF_MEASURED_OVERRIDES(true);

private:
	std::string m_imagePath;
	BoundValue<bool> m_running;
};

// Item-set decode/encode -----------------------------------------------------
// Indices are how every list-shaped wrapper carries a set of items internally,
// and these two functions are the only places the bound type is decoded, so
// every backend -- and both ListBox and CheckListBox -- share one reading of
// it. Free functions rather than members of either wrapper for the reason B6
// gives: the retained backends call them from event handlers and idle syncs,
// which must not capture a wrapper (wrapper and window teardown order is not
// fixed, see wx/RefSync.hpp), only the item list they copy.

// Item indices the control should show for `value`. Out-of-range entries are
// dropped rather than clamped: a stale index means "not in this list", and
// silently selecting a neighbour would be worse than selecting nothing.
template <ListBoxValue T>
std::vector<int> listIndicesFor(const std::vector<std::string>& items, const T& value)
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

	if constexpr (MultiSelectListBoxValue<T>)
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

// The inverse: the bound value for a set of indices. A single-value binding
// with nothing selected reports -1 / "" -- the same "no selection"
// wxNOT_FOUND spelling the retained backends use.
template <ListBoxValue T>
T listValueFor(const std::vector<std::string>& items, const std::vector<int>& indices)
{
	const auto itemAt = [&](int i) -> ListBoxItem<T> {
		if constexpr (std::is_same_v<ListBoxItem<T>, int>)
			return i;
		else
			return (i >= 0 && i < static_cast<int>(items.size())) ? items[i] : std::string{};
	};

	if constexpr (MultiSelectListBoxValue<T>)
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

// ListBoxWrapper -----------------------------------------------------------
// Single- or multi-select depending on T (see ListBoxValue). The selection is
// carried as item indices everywhere inside the wrapper, decoded through the
// shared pair above.
template <ListBoxValue T>
class ListBoxWrapper : public ControlWrapper
{
public:
	static constexpr bool kMultiSelect = MultiSelectListBoxValue<T>;

	ListBoxWrapper(BoundValue<ItemList> items,
		BoundValue<T> selected, int visibleRows, const Position& pos, const Size& size, long style,
		EventCallback<const T&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_items(std::move(items))
		, m_visibleRows(visibleRows)
		, m_value(std::move(selected))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	// Bound items: answered with the size captured at realize() -- the native
	// best size would follow every repopulation (see ItemList).
	DECLARE_SELF_MEASURED_OVERRIDES(m_items.isBound());

	// The backends reach the shared decode/encode pair through these names.
	static std::vector<int> indicesFor(const std::vector<std::string>& items, const T& value)
	{
		return listIndicesFor<T>(items, value);
	}

	static T valueFor(const std::vector<std::string>& items, const std::vector<int>& indices)
	{
		return listValueFor<T>(items, indices);
	}

	// Commit a new selection: the value first, then the user callback, so a
	// handler reading the bound value sees the new one.
	void commit(const std::vector<int>& indices)
	{
		m_value.set(valueFor(m_items.get(), indices));
		m_onChange(m_value.get(), m_nativeWidget);
	}

	const T& boundValue() const { return m_value.get(); }

private:
	BoundValue<ItemList> m_items;
	int m_visibleRows = 1;
	BoundValue<T> m_value;
	EventCallback<const T&> m_onChange;
	Size m_initialSize { 0, 0 }; // wx/Qt, bound items only: the first list's size
};

extern template class ListBoxWrapper<int>;
extern template class ListBoxWrapper<std::string>;
extern template class ListBoxWrapper<std::vector<int>>;
extern template class ListBoxWrapper<std::vector<std::string>>;

// CheckListBoxWrapper -----------------------------------------------------------
// A list with a checkbox per row. The CHECKED SET is the value -- always a
// vector (see CheckListValue) -- and it is carried as indices inside the
// wrapper, decoded through the same listIndicesFor()/listValueFor() pair
// ListBoxWrapper uses. Highlight selection is whatever the native list does
// with a click and is deliberately not part of the value: a row can be
// highlighted without being ticked, and the caller only ever asked about ticks.
template <CheckListValue T>
class CheckListBoxWrapper : public ControlWrapper
{
public:
	CheckListBoxWrapper(BoundValue<ItemList> items,
		BoundValue<T> checked, int visibleRows, const Position& pos, const Size& size, long style,
		EventCallback<const T&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_items(std::move(items))
		, m_visibleRows(visibleRows)
		, m_value(std::move(checked))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	// Bound items: answered with the size captured at realize() -- the native
	// best size would follow every repopulation (see ItemList).
	DECLARE_SELF_MEASURED_OVERRIDES(m_items.isBound());

	static std::vector<int> indicesFor(const std::vector<std::string>& items, const T& value)
	{
		return listIndicesFor<T>(items, value);
	}

	static T valueFor(const std::vector<std::string>& items, const std::vector<int>& indices)
	{
		return listValueFor<T>(items, indices);
	}

	// Commit a new checked set: the value first, then the user callback, so a
	// handler reading the bound value sees the new one.
	void commit(const std::vector<int>& indices)
	{
		m_value.set(valueFor(m_items.get(), indices));
		m_onChange(m_value.get(), m_nativeWidget);
	}

	const T& boundValue() const { return m_value.get(); }

private:
	BoundValue<ItemList> m_items;
	int m_visibleRows = 1;
	BoundValue<T> m_value;
	EventCallback<const T&> m_onChange;
	Size m_initialSize { 0, 0 }; // wx/Qt, bound items only: the first list's size
};

extern template class CheckListBoxWrapper<std::vector<int>>;
extern template class CheckListBoxWrapper<std::vector<std::string>>;

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

	TreeViewWrapper(BoundValue<std::vector<TreeItem>> items,
		BoundValue<T> selected, int visibleRows, bool multiSelect,
		const Position& pos, const Size& size, long style,
		EventCallback<const T&> onChange = {})
		: ControlWrapper(pos, size, style)
		, m_items(std::move(items))
		, m_visibleRows(visibleRows)
		, m_multiSelect(multiSelect)
		, m_value(std::move(selected))
		, m_onChange(std::move(onChange))
	{
	}

	DECLARE_CONTROL_WRAPPER_OVERRIDES();
	// Bound items: answered with the size captured at realize(), as a bound
	// ListBox is -- a refilled tree must not resize an auto-fit window.
	DECLARE_SELF_MEASURED_OVERRIDES(m_items.isBound());

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
		m_onChange(m_value.get(), m_nativeWidget);
	}

	const T& boundValue() const { return m_value.get(); }

	// Paths a refill should show open: an item that was already in the tree
	// keeps what the user left it at (`openBefore` holds the paths that were
	// open, `before` every path that existed), and a new item takes its own
	// `expanded` flag. Shared by wx and Qt; ImGui's own open state behaves
	// this way by itself, keyed by label.
	static std::vector<std::string> openAfterRefill(const std::vector<TreeItem>& next,
		const std::vector<std::string>& before, const std::vector<std::string>& openBefore)
	{
		const auto contains = [](const std::vector<std::string>& paths, const std::string& path) {
			return std::find(paths.begin(), paths.end(), path) != paths.end();
		};
		std::vector<std::string> open;
		forEachItem(next, [&](const TreeItem& item, const std::string& path, int) {
			if (item.children.empty())
				return;
			if (contains(before, path) ? contains(openBefore, path) : item.expanded)
				open.push_back(path);
		});
		return open;
	}

private:
	BoundValue<std::vector<TreeItem>> m_items;
	int m_visibleRows = 1;
	bool m_multiSelect = false;
	BoundValue<T> m_value;
	EventCallback<const T&> m_onChange;
	Size m_initialSize { 0, 0 }; // wx/Qt, bound items only: the first tree's size
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
// every backend redraws from; unbound, it lands in a snapshot and only
// onCellChange observes it. The edit survives either way on every backend --
// the native control retains it on wx and Qt, and on ImGui, where the whole
// tree is rebuilt every frame, an unbound snapshot lives in the per-widget
// store (frameworks_core/imgui/SnapshotStore.hpp) rather than in this wrapper.
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
		EventCallback<const T&> onChange = {},
		std::function<void(int, int, const std::string&)> onCellChange = {})
		: ControlWrapper(pos, size, style)
		, m_columns(std::move(columns))
		, m_rows(std::move(rows))
		, m_visibleRows(visibleRows)
		, m_value(std::move(selected))
		, m_onChange(std::move(onChange))
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

	// `rows` cut or padded to exactly `columnCount` cells each -- what a native
	// table actually holds, since cellText() reads a ragged row as trailing
	// empty cells. The retained backends compare the caller's rows with the
	// control's in this shape, so a short row is not a perpetual difference.
	static TableRows normalizedRows(const TableRows& rows, int columnCount)
	{
		TableRows out(rows.size(), TableRow(static_cast<std::size_t>(columnCount)));
		for (int row = 0; row < static_cast<int>(rows.size()); ++row)
		{
			for (int column = 0; column < columnCount; ++column)
				out[row][column] = cellText(rows, row, column);
		}
		return out;
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
		markChanged(rows); // counted when the rows are an Observable's
	}

	// Commit a new selection: the value first, then the user callback, so a
	// handler reading the bound value sees the new one.
	void commit(const std::vector<int>& indices)
	{
		m_value.set(valueFor(m_rows.get(), indices));
		m_onChange(m_value.get(), m_nativeWidget);
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
	// write through to. A snapshot reports nullptr -- an edit still sticks, but
	// it is the framework's copy that keeps it.
	TableRows* boundRows() { return m_rows.isBound() ? &m_rows.get() : nullptr; }

private:
	std::vector<TableColumn> m_columns;
	BoundValue<TableRows> m_rows;
	int m_visibleRows = 1;
	BoundValue<T> m_value;
	EventCallback<const T&> m_onChange;
	std::function<void(int, int, const std::string&)> m_onCellChange;

	// ImGui: column widths measureIntrinsic() computed this frame, reused by
	// render() on the same wrapper so every cell's text is measured once per
	// frame, not twice. Empty until measured, and unused on wx/Qt.
	std::vector<int> m_columnWidths;
};

extern template class TableWrapper<int>;
extern template class TableWrapper<std::string>;
extern template class TableWrapper<std::vector<int>>;
extern template class TableWrapper<std::vector<std::string>>;
