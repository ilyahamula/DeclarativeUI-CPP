#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

#include "buildable.hpp"

template<NodeBuildable... W>
struct GroupBox
{
	GroupBox(Orientation orient, const std::string& label, W... widgets)
		: m_orient(orient)
		, m_label(label)
		, m_widgets(std::make_tuple(std::move(widgets)...))
	{
	}

	GroupBox(Orientation orient, const std::string& label, LayoutFlags flags, W... widgets)
		: m_orient(orient)
		, m_label(label)
		, m_flags(flags)
		, m_widgets(std::make_tuple(std::move(widgets)...))
	{
	}

	// Disable the box and, with it, every widget inside it -- the title greys
	// out too. Snapshot the flag as it stands now.
	GroupBox& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
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

	// Take the whole subtree out of the layout -- see Widget::isHidden().
	GroupBox& isHidden(const bool& hidden = true)
	{
		m_hidden.snapshot(hidden);
		return *this;
	}

	GroupBox& isHidden(bool& hidden)
	{
		m_hidden.bind(hidden);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeGroupBox(m_orient, m_label, m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		node->hidden = m_hidden;
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
	BoundValue<bool> m_hidden { false };
	std::tuple<W...> m_widgets;
};

template<NodeBuildable... W>
struct HGroupBox : public GroupBox<W...>
{
	HGroupBox(W... widgets)
		: GroupBox<W...>(Orientation::Horizontal, "", std::move(widgets)...)
	{
	}

	HGroupBox(LayoutFlags flags, W... widgets)
		: GroupBox<W...>(Orientation::Horizontal, "", flags, std::move(widgets)...)
	{
	}

	HGroupBox(const std::string& label, W... widgets)
		: GroupBox<W...>(Orientation::Horizontal, label, std::move(widgets)...)
	{
	}

	HGroupBox(const std::string& label, LayoutFlags flags, W... widgets)
		: GroupBox<W...>(Orientation::Horizontal, label, flags, std::move(widgets)...)
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

	// Take the whole subtree out of the layout -- see Widget::isHidden().
	HGroupBox& isHidden(const bool& hidden = true)
	{
		GroupBox<W...>::isHidden(hidden);
		return *this;
	}

	HGroupBox& isHidden(bool& hidden)
	{
		GroupBox<W...>::isHidden(hidden);
		return *this;
	}
};

template<NodeBuildable... W>
struct VGroupBox : public GroupBox<W...>
{
	VGroupBox(W... widgets)
		: GroupBox<W...>(Orientation::Vertical, "", std::move(widgets)...)
	{
	}

	VGroupBox(LayoutFlags flags, W... widgets)
		: GroupBox<W...>(Orientation::Vertical, "", flags, std::move(widgets)...)
	{
	}

	VGroupBox(const std::string& label, W... widgets)
		: GroupBox<W...>(Orientation::Vertical, label, std::move(widgets)...)
	{
	}

	VGroupBox(const std::string& label, LayoutFlags flags, W... widgets)
		: GroupBox<W...>(Orientation::Vertical, label, flags, std::move(widgets)...)
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

	// Take the whole subtree out of the layout -- see Widget::isHidden().
	VGroupBox& isHidden(const bool& hidden = true)
	{
		GroupBox<W...>::isHidden(hidden);
		return *this;
	}

	VGroupBox& isHidden(bool& hidden)
	{
		GroupBox<W...>::isHidden(hidden);
		return *this;
	}
};
