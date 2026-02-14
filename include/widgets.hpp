#pragma once

#include "frameworks_core/ControlWrappers.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <vector>

template <typename W>
struct Widget
{
	Widget(std::string str)
		: m_str(str)
	{
	}

	virtual ~Widget() = default;

	void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
	{
		auto control = createWrapper(parent, m_str, m_position, m_size, m_style);
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
		const std::string& str,
		const Position& pos,
		const Size& size,
		long style) = 0;

private:
	std::string m_str;
	std::optional<LayoutFlags> m_flags;
	Position m_position { -1, -1 };
	Size m_size { -1, -1 };
	long m_style { 0 };
};

// StaticText -----------------------------------------------------------
struct StaticText : Widget<StaticText>
{
	using super = Widget<StaticText>;

	explicit StaticText(const std::string& str)
		: super(str)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const std::string& str,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<StaticTextWrapper>(parent, str, pos, size, style);
	}
};


// TextCtrl -----------------------------------------------------------
struct TextCtrl : Widget<TextCtrl>
{
	using super = Widget<TextCtrl>;

	explicit TextCtrl(const std::string& str = "")
		: super(str)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const std::string& str,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<TextCtrlWrapper>(parent, str, pos, size, style);
	}
};

// Button -----------------------------------------------------------
struct Button : Widget<Button>
{
	using super = Widget<Button>;

	explicit Button(const std::string& str = "")
		: super(str)
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
		const std::string& str,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ButtonWrapper>(parent, str, pos, size, style, m_onClick);
	}

private:
	std::function<void()> m_onClick;
};

// RadioButton -----------------------------------------------------------
struct RadioButton : Widget<RadioButton>
{
	using super = Widget<RadioButton>;

	explicit RadioButton(const std::string& str = "")
		: super(str)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const std::string& str,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<RadioButtonWrapper>(parent, str, pos, size, style);
	}
};

// CheckBox -----------------------------------------------------------
struct CheckBox : Widget<CheckBox>
{
	using super = Widget<CheckBox>;

	explicit CheckBox(const std::string& str = "")
		: super(str)
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
		const std::string& str,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<CheckBoxWrapper>(parent, str, pos, size, style, m_checked);
	}

private:
	bool m_checked { false };
};

// ComboBox -----------------------------------------------------------
struct ComboBox : Widget<ComboBox>
{
	using super = Widget<ComboBox>;

	explicit ComboBox(std::vector<std::string> choices, const std::string& selected = "")
		: super(selected)
		, m_choices(std::move(choices))
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const std::string& str,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<ComboBoxWrapper>(parent, m_choices, str, pos, size, style);
	}

private:
	std::vector<std::string> m_choices;
};

// Slider -----------------------------------------------------------
struct Slider : Widget<Slider>
{
	using super = Widget<Slider>;

	explicit Slider(Range range)
		: super("")
		, m_range(range)
	{
	}

private:
	std::unique_ptr<ControlWrapper> createWrapper(
		ControlWrapper* parent,
		const std::string& str,
		const Position& pos,
		const Size& size,
		long style) override
	{
		return std::make_unique<SliderWrapper>(parent, m_range, pos, size, style);
	}

private:
	Range m_range;
};
