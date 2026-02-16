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

	explicit TextCtrl(std::string& text)
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
		return std::make_unique<TextCtrlWrapper>(parent, m_text, pos, size, style);
	}

private:
	std::string& m_text;
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

	RadioButton(T& value, const std::string& label = "")
		: super()
		, m_label(label)
		, m_value(value)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<RadioButtonWrapper<T>>(parent, m_label, m_value, pos, size, style);
	}

private:
	std::string m_label;
	T& m_value;
};

template <RadioButtonValue T>
RadioButton(T&) -> RadioButton<T>;

template <RadioButtonValue T>
RadioButton(T&, const std::string&) -> RadioButton<T>;

// CheckBox -----------------------------------------------------------
struct CheckBox : Widget<CheckBox>
{
	using super = Widget<CheckBox>;

	explicit CheckBox(bool& checked, const std::string& str = "")
		: super()
		, m_checked(checked)
		, m_label(str)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<CheckBoxWrapper>(parent, m_label, pos, size, style, m_checked);
	}

private:
	bool& m_checked;
	std::string m_label;
};

// ComboBox -----------------------------------------------------------
template <ComboBoxValue T>
struct ComboBox : Widget<ComboBox<T>>
{
	using super = Widget<ComboBox<T>>;

	ComboBox(std::vector<std::string> choices, T& selected)
		: super()
		, m_choices(std::move(choices))
		, m_selected(selected)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ComboBoxWrapper<T>>(parent, m_choices, m_selected, pos, size, style);
	}

private:
	std::vector<std::string> m_choices;
	T& m_selected;
};

template <ComboBoxValue T>
ComboBox(std::vector<std::string>, T&) -> ComboBox<T>;

// Slider -----------------------------------------------------------
template <SliderValue T>
struct Slider : Widget<Slider<T>>
{
	using super = Widget<Slider<T>>;

	Slider(Range<T> range, T& value)
		: super()
		, m_range(range)
		, m_value(value)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<SliderWrapper<T>>(parent, m_range, m_value, pos, size, style);
	}

private:
	Range<T> m_range;
	T& m_value;
};

template <SliderValue T>
Slider(Range<T>, T) -> Slider<T>;
