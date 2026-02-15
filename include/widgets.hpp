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
struct RadioButton : Widget<RadioButton>
{
	using super = Widget<RadioButton>;

	explicit RadioButton(const std::string& str = "")
		: super()
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
		return std::make_unique<RadioButtonWrapper>(parent, m_label, pos, size, style);
	}

private:
	std::string m_label;
};

// CheckBox -----------------------------------------------------------
struct CheckBox : Widget<CheckBox>
{
	using super = Widget<CheckBox>;

	explicit CheckBox(const std::string& str = "")
		: super()
		, m_label(str)
	{
	}

	CheckBox& setChecked(bool value = true)
	{
		m_checked = value;
		return *this;
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
	bool m_checked { false };
	std::string m_label;
};

// ComboBox -----------------------------------------------------------
struct ComboBox : Widget<ComboBox>
{
	using super = Widget<ComboBox>;

	explicit ComboBox(std::vector<std::string> choices, const std::string& selected = "")
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
		return std::make_unique<ComboBoxWrapper>(parent, m_choices, m_selected, pos, size, style);
	}

private:
	std::vector<std::string> m_choices;
	std::string m_selected;
};

// Slider -----------------------------------------------------------
template <SliderValue T>
struct Slider : Widget<Slider<T>>
{
	using super = Widget<Slider<T>>;

	Slider(Range<T> range, T& value)
		: super()
		, m_range(range)
		, m_ptrValue(&value)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<SliderWrapper<T>>(parent, m_range, m_ptrValue, pos, size, style);
	}

private:
	Range<T> m_range;
	T* m_ptrValue;
};

template <SliderValue T>
Slider(Range<T>, T&) -> Slider<T>;
