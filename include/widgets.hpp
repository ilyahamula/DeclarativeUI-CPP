#pragma once

#include "frameworks_core/ControlWrappers.hpp"
#include "create_and_add.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <vector>

template <typename W>
struct Widget
{
	Widget() = default;

	virtual ~Widget() = default;

	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
	{
		auto control = createWrapper(parent, m_position, m_size, m_style);
		control->createAndAdd(parent, layout, m_flags.value_or(flags));
	}

	W& withFlags(LayoutFlags flags)
	{
		m_flags = flags;
		return static_cast<W&>(*this);
	}

	W& withPosition(const Position& pos)
	{
		m_position = pos;
		return static_cast<W&>(*this);
	}

	W& withSize(const Size& size)
	{
		m_size = size;
		return static_cast<W&>(*this);
	}

	W& withStyle(long style)
	{
		m_style = style;
		return static_cast<W&>(*this);
	}

private:
	virtual std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) = 0;

private:
	std::optional<LayoutFlags> m_flags;
	Position m_position { -1, -1 };
	Size m_size { -1, -1 };
	long m_style { 0 };
};

// StaticText -----------------------------------------------------------
struct StaticText : Widget<StaticText>
{
	using super = Widget<StaticText>;

	explicit StaticText(const std::string& text)
		: super()
		, m_text(text)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<StaticTextWrapper>(parent, m_text, pos, size, style);
	}

private:
	std::string m_text;
};

// TextCtrl -----------------------------------------------------------
struct TextCtrl : Widget<TextCtrl>
{
	using super = Widget<TextCtrl>;

	TextCtrl()
		: super()
	{
	}

	explicit TextCtrl(const std::string& text)
		: super()
		, m_ownedText(text)
	{
	}

	explicit TextCtrl(std::string& text)
		: super()
		, m_ownedText(text)
		, m_externalRef(text)
	{
	}

	TextCtrl& onChange(std::function<void(const std::string&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<TextCtrlWrapper>(parent, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<TextCtrlWrapper>(parent, m_ownedText, pos, size, style, m_onChange);
	}

private:
	std::string m_ownedText;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
};

// PasswordInput -----------------------------------------------------------
struct PasswordInput : Widget<PasswordInput>
{
	using super = Widget<PasswordInput>;

	PasswordInput()
		: super()
	{
	}

	explicit PasswordInput(const std::string& text)
		: super()
		, m_ownedText(text)
	{
	}

	explicit PasswordInput(std::string& text)
		: super()
		, m_ownedText(text)
		, m_externalRef(text)
	{
	}

	PasswordInput& onChange(std::function<void(const std::string&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<PasswordInputWrapper>(parent, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<PasswordInputWrapper>(parent, m_ownedText, pos, size, style, m_onChange);
	}

private:
	std::string m_ownedText;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
};

// MultiLineTextCtrl -----------------------------------------------------------
struct MultiLineTextCtrl : Widget<MultiLineTextCtrl>
{
	using super = Widget<MultiLineTextCtrl>;

	MultiLineTextCtrl()
		: super()
	{
	}

	explicit MultiLineTextCtrl(const std::string& text)
		: super()
		, m_ownedText(text)
	{
	}

	explicit MultiLineTextCtrl(std::string& text)
		: super()
		, m_ownedText(text)
		, m_externalRef(text)
	{
	}

	MultiLineTextCtrl& onChange(std::function<void(const std::string&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<MultiLineTextCtrlWrapper>(parent, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<MultiLineTextCtrlWrapper>(parent, m_ownedText, pos, size, style, m_onChange);
	}

private:
	std::string m_ownedText;
	std::optional<std::reference_wrapper<std::string>> m_externalRef;
	std::function<void(const std::string&)> m_onChange;
};

// ReadonlyTextCtrl -----------------------------------------------------------
struct ReadonlyTextCtrl : Widget<ReadonlyTextCtrl>
{
	using super = Widget<ReadonlyTextCtrl>;

	explicit ReadonlyTextCtrl(const std::string& text)
		: super()
		, m_text(text)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ReadonlyTextCtrlWrapper>(parent, m_text, pos, size, style);
	}

private:
	const std::string& m_text;
};

// ClickableText -----------------------------------------------------------
struct ClickableText : Widget<ClickableText>
{
	using super = Widget<ClickableText>;

	explicit ClickableText(const std::string& text)
		: super()
		, m_text(text)
	{
	}

	ClickableText& onClick(std::function<void()> callback)
	{
		m_onClick = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ClickableTextWrapper>(parent, m_text, pos, size, style, m_onClick);
	}

private:
	std::function<void()> m_onClick;
	std::string m_text;
};

// LinkText -----------------------------------------------------------
struct LinkText : Widget<LinkText>
{
	using super = Widget<LinkText>;

	explicit LinkText(const std::string& text)
		: super()
		, m_text(text)
	{
	}

	LinkText& onClick(std::function<void()> callback)
	{
		m_onClick = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<LinkTextWrapper>(parent, m_text, pos, size, style, m_onClick);
	}

private:
	std::function<void()> m_onClick;
	std::string m_text;
};

// Button -----------------------------------------------------------
struct Button : Widget<Button>
{
	using super = Widget<Button>;

	explicit Button(const std::string& str = "")
		: super()
		, m_btnTitle(str)
	{
	}

	Button& onClick(std::function<void()> callback)
	{
		m_onClick = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ButtonWrapper>(parent, m_btnTitle, pos, size, style, m_onClick);
	}

private:
	std::function<void()> m_onClick;
	std::string m_btnTitle;
};

// RadioButton -----------------------------------------------------------
template <RadioButtonValue T>
struct RadioButton : Widget<RadioButton<T>>
{
	using super = Widget<RadioButton<T>>;

	RadioButton()
		: super()
	{
	}

	RadioButton(const T& value, const std::string& label = "")
		: super()
		, m_ownedValue(value)
		, m_label(label)
	{
	}

	RadioButton(T& value, const std::string& label = "")
		: super()
		, m_ownedValue(value)
		, m_externalRef(value)
		, m_label(label)
	{
	}

	RadioButton& onChange(std::function<void(T)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<RadioButtonWrapper<T>>(parent, m_label, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<RadioButtonWrapper<T>>(parent, m_label, m_ownedValue, pos, size, style, m_onChange);
	}

private:
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::string m_label;
	std::function<void(T)> m_onChange;
};

template <RadioButtonValue T>
RadioButton(T&) -> RadioButton<T>;

template <RadioButtonValue T>
RadioButton(T&, const std::string&) -> RadioButton<T>;

template <RadioButtonValue T>
RadioButton(const T&) -> RadioButton<T>;

template <RadioButtonValue T>
RadioButton(const T&, const std::string&) -> RadioButton<T>;

// CheckBox -----------------------------------------------------------
struct CheckBox : Widget<CheckBox>
{
	using super = Widget<CheckBox>;

	CheckBox()
		: super()
	{
	}

	CheckBox(const bool& initialChecked, const std::string& label = "")
		: super()
		, m_ownedValue(initialChecked)
		, m_label(label)
	{
	}

	CheckBox(bool& checked, const std::string& label = "")
		: super()
		, m_ownedValue(checked)
		, m_externalRef(checked)
		, m_label(label)
	{
	}

	CheckBox& onChange(std::function<void(bool)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<CheckBoxWrapper>(parent, m_label, pos, size, style, m_externalRef->get(), m_onChange);
		return std::make_unique<CheckBoxWrapper>(parent, m_label, pos, size, style, std::as_const(m_ownedValue), m_onChange);
	}

private:
	bool m_ownedValue = false;
	std::optional<std::reference_wrapper<bool>> m_externalRef;
	std::string m_label;
	std::function<void(bool)> m_onChange;
};

// ComboBox -----------------------------------------------------------
template <ComboBoxValue T>
struct ComboBox : Widget<ComboBox<T>>
{
	using super = Widget<ComboBox<T>>;

	ComboBox(std::vector<std::string> choices)
		: super()
		, m_choices(std::move(choices))
	{
		if constexpr (std::is_same_v<T, std::string>)
			m_ownedSelected = m_choices.empty() ? T{} : m_choices.front();
	}

	ComboBox(std::vector<std::string> choices, const T& selected)
		: super()
		, m_choices(std::move(choices))
		, m_ownedSelected(selected)
	{
	}

	ComboBox(std::vector<std::string> choices, T& selected)
		: super()
		, m_choices(std::move(choices))
		, m_ownedSelected(selected)
		, m_externalRef(selected)
	{
	}

	ComboBox& onChange(std::function<void(const T&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<ComboBoxWrapper<T>>(parent, m_choices, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<ComboBoxWrapper<T>>(parent, m_choices, std::as_const(m_ownedSelected), pos, size, style, m_onChange);
	}

private:
	std::vector<std::string> m_choices;
	T m_ownedSelected{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(const T&)> m_onChange;
};

template <ComboBoxValue T>
ComboBox(std::vector<std::string>, T&) -> ComboBox<T>;

template <ComboBoxValue T>
ComboBox(std::vector<std::string>, const T&) -> ComboBox<T>;

// Slider -----------------------------------------------------------
template <SliderValue T>
struct Slider : Widget<Slider<T>>
{
	using super = Widget<Slider<T>>;

	explicit Slider(Range<T> range)
		: super()
		, m_range(range)
		, m_ownedValue(range.min)
	{
	}

	Slider(Range<T> range, const T& value)
		: super()
		, m_range(range)
		, m_ownedValue(value)
	{
	}

	Slider(Range<T> range, T& value)
		: super()
		, m_range(range)
		, m_ownedValue(value)
		, m_externalRef(value)
	{
	}

	Slider& onChange(std::function<void(T)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<SliderWrapper<T>>(parent, m_range, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<SliderWrapper<T>>(parent, m_range, std::as_const(m_ownedValue), pos, size, style, m_onChange);
	}

private:
	Range<T> m_range;
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
};

template <SliderValue T>
Slider(Range<T>) -> Slider<T>;

template <SliderValue T>
Slider(Range<T>, T&) -> Slider<T>;

template <SliderValue T>
Slider(Range<T>, const T&) -> Slider<T>;

// SpinBox -----------------------------------------------------------
template <SpinBoxValue T>
struct SpinBox : Widget<SpinBox<T>>
{
	using super = Widget<SpinBox<T>>;

	explicit SpinBox(Range<T> range)
		: super()
		, m_range(range)
		, m_ownedValue(range.min)
	{
	}

	SpinBox(Range<T> range, const T& value)
		: super()
		, m_range(range)
		, m_ownedValue(value)
	{
	}

	SpinBox(Range<T> range, T& value)
		: super()
		, m_range(range)
		, m_ownedValue(value)
		, m_externalRef(value)
	{
	}

	SpinBox& onChange(std::function<void(T)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<SpinBoxWrapper<T>>(parent, m_range, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<SpinBoxWrapper<T>>(parent, m_range, std::as_const(m_ownedValue), pos, size, style, m_onChange);
	}

private:
	Range<T> m_range;
	T m_ownedValue{};
	std::optional<std::reference_wrapper<T>> m_externalRef;
	std::function<void(T)> m_onChange;
};

template <SpinBoxValue T>
SpinBox(Range<T>) -> SpinBox<T>;

template <SpinBoxValue T>
SpinBox(Range<T>, T&) -> SpinBox<T>;

template <SpinBoxValue T>
SpinBox(Range<T>, const T&) -> SpinBox<T>;

// DatePicker -----------------------------------------------------------
struct DatePicker : Widget<DatePicker>
{
	using super = Widget<DatePicker>;

	DatePicker()
		: super()
	{
	}

	explicit DatePicker(const Date& value)
		: super()
		, m_ownedValue(value)
	{
	}

	explicit DatePicker(Date& value)
		: super()
		, m_ownedValue(value)
		, m_externalRef(value)
	{
	}

	DatePicker& onChange(std::function<void(const Date&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<DatePickerWrapper>(parent, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<DatePickerWrapper>(parent, std::as_const(m_ownedValue), pos, size, style, m_onChange);
	}

private:
	Date m_ownedValue{};
	std::optional<std::reference_wrapper<Date>> m_externalRef;
	std::function<void(const Date&)> m_onChange;
};

// ToggleButton -----------------------------------------------------------
struct ToggleButton : Widget<ToggleButton>
{
	using super = Widget<ToggleButton>;

	ToggleButton()
		: super()
	{
	}

	ToggleButton(const bool& initialToggled, const std::string& label = "")
		: super()
		, m_ownedValue(initialToggled)
		, m_label(label)
	{
	}

	ToggleButton(bool& toggled, const std::string& label = "")
		: super()
		, m_ownedValue(toggled)
		, m_externalRef(toggled)
		, m_label(label)
	{
	}

	ToggleButton& onChange(std::function<void(bool)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<ToggleButtonWrapper>(parent, m_label, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<ToggleButtonWrapper>(parent, m_label, m_ownedValue, pos, size, style, m_onChange);
	}

private:
	std::string m_label;
	bool m_ownedValue = false;
	std::optional<std::reference_wrapper<bool>> m_externalRef;
	std::function<void(bool)> m_onChange;
};

// Separator -----------------------------------------------------------
struct Separator : Widget<Separator>
{
	using super = Widget<Separator>;

	Separator() : super() {}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<SeparatorWrapper>(parent, pos, size, style);
	}
};

// ProgressBar -----------------------------------------------------------
struct ProgressBar : Widget<ProgressBar>
{
	using super = Widget<ProgressBar>;

	explicit ProgressBar(const float& value)
		: super()
		, m_ownedValue(value)
	{
	}

	explicit ProgressBar(float& value)
		: super()
		, m_ownedValue(value)
		, m_externalRef(value)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<ProgressBarWrapper>(parent, m_externalRef->get(), pos, size, style);
		return std::make_unique<ProgressBarWrapper>(parent, std::as_const(m_ownedValue), pos, size, style);
	}

private:
	float m_ownedValue = 0.0f;
	std::optional<std::reference_wrapper<float>> m_externalRef;
};

// Image -----------------------------------------------------------
struct Image : Widget<Image>
{
	using super = Widget<Image>;

	explicit Image(const std::string& filePath)
		: super()
		, m_filePath(filePath)
	{
	}

	Image& onClick(std::function<void()> callback)
	{
		m_onClick = std::move(callback);
		return *this;
	}

	Image& onHover(std::function<void()> callback)
	{
		m_onHover = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ImageWrapper>(parent, m_filePath, pos, size, style, m_onClick, m_onHover);
	}

private:
	std::string m_filePath;
	std::function<void()> m_onClick;
	std::function<void()> m_onHover;
};

// TimePicker -----------------------------------------------------------
struct TimePicker : Widget<TimePicker>
{
	using super = Widget<TimePicker>;

	TimePicker()
		: super()
	{
	}

	explicit TimePicker(const Time& value)
		: super()
		, m_ownedValue(value)
	{
	}

	explicit TimePicker(Time& value)
		: super()
		, m_ownedValue(value)
		, m_externalRef(value)
	{
	}

	TimePicker& onChange(std::function<void(const Time&)> callback)
	{
		m_onChange = std::move(callback);
		return *this;
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		if (m_externalRef)
			return std::make_unique<TimePickerWrapper>(parent, m_externalRef->get(), pos, size, style, m_onChange);
		return std::make_unique<TimePickerWrapper>(parent, std::as_const(m_ownedValue), pos, size, style, m_onChange);
	}

private:
	Time m_ownedValue{};
	std::optional<std::reference_wrapper<Time>> m_externalRef;
	std::function<void(const Time&)> m_onChange;
};
