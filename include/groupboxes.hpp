#pragma once

#include <memory>
#include <optional>
#include <tuple>

#include "frameworks_core/GroupBoxWrapper.hpp"
#include "create_and_add.hpp"

template<CreateAndAddable... W>
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

	std::unique_ptr<GroupBoxWrapper> createAndAdd(ControlWrapper* parent, LayoutFlags parentFlags)
	{
		auto box = std::make_unique<GroupBoxWrapper>(parent, m_orient, m_label);
		::createAndAdd(box->staticBox(), box.get(), m_flags.value_or(parentFlags), m_widgets);
		box->finilizeLayout();
		return box;
	}

	auto createAndAdd(ControlWrapper* parent, LayoutWrapper* parentLayout, LayoutFlags parentFlags)
	{
		parentLayout->add(parentFlags);
		auto box = createAndAdd(parent, parentFlags);
		parentLayout->add(box.get(), parentFlags);
		return box;
	}

	auto fitTo(ControlWrapper* parent)
	{
		auto box = createAndAdd(parent, m_flags.value_or(LayoutFlags()));
		parent->setLayout(box.get());
		return box;
	}

private:
	Orientation m_orient;
	std::string m_label;
	std::optional<LayoutFlags> m_flags;
	std::tuple<W...> m_widgets;
};

template<CreateAndAddable... W>
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

template<CreateAndAddable... W>
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
