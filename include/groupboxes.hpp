#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>

#include "buildable.hpp"

template<NodeBuildable... W>
struct GroupBox
{
	GroupBox(Orientation orient, const std::string& label, W... widgets)
		: m_orient(orient)
		, m_label(label)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	GroupBox(Orientation orient, const std::string& label, LayoutFlags flags, W... widgets)
		: m_orient(orient)
		, m_label(label)
		, m_flags(flags)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	// Disable the box and, with it, every widget inside it -- the title greys
	// out too. Snapshot the flag as it stands now.
	GroupBox& isDisabled(const bool& disabled = true)
	{
		m_disabled.set(disabled);
		return *this;
	}

	// Bind to a caller-owned flag: flipping it disables/enables the whole
	// subtree without rebuilding the tree. A child cannot opt back out --
	// disabling always cascades down.
	GroupBox& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeGroupBox(m_orient, m_label, m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		std::apply([&](auto&... widget) {
			(node->add(widget.buildNode()), ...);
		}, m_widgets);
		return node;
	}

private:
	Orientation m_orient;
	std::string m_label;
	std::optional<LayoutFlags> m_flags;
	DisabledFlag m_disabled;
	std::tuple<W...> m_widgets;
};

template<NodeBuildable... W>
struct HGroupBox : public GroupBox<W...>
{
	HGroupBox(W... widgets)
		: GroupBox<W...>(Orientation::Horizontal, "", widgets...)
	{
	}

	HGroupBox(LayoutFlags flags, W... widgets)
		: GroupBox<W...>(Orientation::Horizontal, "", flags, widgets...)
	{
	}

	HGroupBox(const std::string& label, W... widgets)
		: GroupBox<W...>(Orientation::Horizontal, label, widgets...)
	{
	}

	HGroupBox(const std::string& label, LayoutFlags flags, W... widgets)
		: GroupBox<W...>(Orientation::Horizontal, label, flags, widgets...)
	{
	}

	// re-declared so chaining keeps the HGroupBox type
	HGroupBox& isDisabled(const bool& disabled = true)
	{
		GroupBox<W...>::isDisabled(disabled);
		return *this;
	}

	HGroupBox& isDisabled(bool& disabled)
	{
		GroupBox<W...>::isDisabled(disabled);
		return *this;
	}
};

template<NodeBuildable... W>
struct VGroupBox : public GroupBox<W...>
{
	VGroupBox(W... widgets)
		: GroupBox<W...>(Orientation::Vertical, "", widgets...)
	{
	}

	VGroupBox(LayoutFlags flags, W... widgets)
		: GroupBox<W...>(Orientation::Vertical, "", flags, widgets...)
	{
	}

	VGroupBox(const std::string& label, W... widgets)
		: GroupBox<W...>(Orientation::Vertical, label, widgets...)
	{
	}

	VGroupBox(const std::string& label, LayoutFlags flags, W... widgets)
		: GroupBox<W...>(Orientation::Vertical, label, flags, widgets...)
	{
	}

	// re-declared so chaining keeps the VGroupBox type
	VGroupBox& isDisabled(const bool& disabled = true)
	{
		GroupBox<W...>::isDisabled(disabled);
		return *this;
	}

	VGroupBox& isDisabled(bool& disabled)
	{
		GroupBox<W...>::isDisabled(disabled);
		return *this;
	}
};
