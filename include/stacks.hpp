#pragma once

#include <memory>
#include <optional>
#include <tuple>

#include "create_and_add.hpp"

template<CreateAndAddable... W>
struct Stack
{
	Stack(Orientation orient, W... widgets)
		: m_orient(orient)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	Stack(Orientation orient, LayoutFlags flags, W... widgets)
		: m_orient(orient)
		, m_flags(flags)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	std::unique_ptr<LayoutWrapper> createAndAdd(ControlWrapper* parent, LayoutFlags parentFlags)
	{
		auto layout = std::make_unique<LayoutWrapper>(m_orient);
		::createAndAdd(parent, layout.get(), m_flags.value_or(parentFlags), m_widgets);
		return layout;
	}

	auto createAndAdd(ControlWrapper* parent, LayoutWrapper* parentLayout, LayoutFlags parentFlags)
	{
		auto layout = createAndAdd(parent, parentFlags);
		parentLayout->add(layout.get(), parentFlags);
		return layout;
	}

	auto fitTo(ControlWrapper* parent)
	{
		auto layout = createAndAdd(parent, m_flags.value_or(LayoutFlags()));
		parent->setLayout(layout.get());
		return layout;
	}

	auto fitTo(ControlWrapper&& parent)
	{
		auto layout = createAndAdd(&parent, m_flags.value_or(LayoutFlags()));
		parent.setLayout(layout.get());
		return layout;
	}

private:
	Orientation m_orient;
	std::optional<LayoutFlags> m_flags;
	std::tuple<W...> m_widgets;
};

template<CreateAndAddable... W>
struct HStack : public Stack<W...>
{
	HStack(W... widgets)
		: Stack<W...>(Orientation::Horizontal, widgets...)
	{
	}

	HStack(LayoutFlags flags, W... widgets)
		: Stack<W...>(Orientation::Horizontal, flags, widgets...)
	{
	}
};

template<CreateAndAddable... W>
struct VStack : public Stack<W...>
{
	VStack(W... widgets)
		: Stack<W...>(Orientation::Vertical, widgets...)
	{
	}

	VStack(LayoutFlags flags, W... widgets)
		: Stack<W...>(Orientation::Vertical, flags, widgets...)
	{
	}
};
