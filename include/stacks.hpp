#pragma once

#include <memory>
#include <optional>
#include <tuple>

#include "buildable.hpp"

template<NodeBuildable... W>
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

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeBox(m_orient, m_flags.value_or(LayoutFlags{}));
		std::apply([&](auto&... widget) {
			(node->add(widget.buildNode()), ...);
		}, m_widgets);
		return node;
	}

private:
	Orientation m_orient;
	std::optional<LayoutFlags> m_flags;
	std::tuple<W...> m_widgets;
};

template<NodeBuildable... W>
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

template<NodeBuildable... W>
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
