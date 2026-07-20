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

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeGroupBox(m_orient, m_label, m_flags.value_or(LayoutFlags{}));
		std::apply([&](auto&... widget) {
			(node->add(widget.buildNode()), ...);
		}, m_widgets);
		return node;
	}

private:
	Orientation m_orient;
	std::string m_label;
	std::optional<LayoutFlags> m_flags;
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
};
