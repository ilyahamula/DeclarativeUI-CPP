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

	// Disable the stack and, with it, every widget inside it. Snapshot the flag
	// as it stands now.
	Stack& isDisabled(const bool& disabled = true)
	{
		m_disabled.set(disabled);
		return *this;
	}

	// Bind to a caller-owned flag: flipping it disables/enables the whole
	// subtree without rebuilding the tree. A child cannot opt back out --
	// disabling always cascades down.
	Stack& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeBox(m_orient, m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		std::apply([&](auto&... widget) {
			(node->add(widget.buildNode()), ...);
		}, m_widgets);
		return node;
	}

private:
	Orientation m_orient;
	std::optional<LayoutFlags> m_flags;
	DisabledFlag m_disabled;
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

	// re-declared so chaining keeps the HStack type
	HStack& isDisabled(const bool& disabled = true)
	{
		Stack<W...>::isDisabled(disabled);
		return *this;
	}

	HStack& isDisabled(bool& disabled)
	{
		Stack<W...>::isDisabled(disabled);
		return *this;
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

	// re-declared so chaining keeps the VStack type
	VStack& isDisabled(const bool& disabled = true)
	{
		Stack<W...>::isDisabled(disabled);
		return *this;
	}

	VStack& isDisabled(bool& disabled)
	{
		Stack<W...>::isDisabled(disabled);
		return *this;
	}
};
