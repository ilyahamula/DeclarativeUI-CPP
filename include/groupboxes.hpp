#pragma once

#include <wx/wx.h>

#include <optional>
#include <tuple>

#include "create_and_add.hpp"

template<CreateAndAddable... W>
struct GroupBox
{
	GroupBox(wxOrientation orient, const std::string& label, W... widgets)
		: m_orient(orient)
		, m_label(label)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	GroupBox(wxOrientation orient, const std::string& label, wxSizerFlags flags, W... widgets)
		: m_orient(orient)
		, m_label(label)
		, m_flags(flags)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	auto createAndAdd(wxWindow* parent, wxSizerFlags parentFlags)
	{
		auto* box = new wxStaticBoxSizer(m_orient, parent, m_label);
		::createAndAdd(box->GetStaticBox(), box, m_flags.value_or(parentFlags), m_widgets);
		return box;
	}

	auto createAndAdd(wxWindow* parent, wxSizer* parentSizer, wxSizerFlags parentFlags)
	{
		auto* box = createAndAdd(parent, parentFlags);
		parentSizer->Add(box, parentFlags);
		return box;
	}

	auto fitTo(wxWindow* parent)
	{
		auto* box = createAndAdd(parent, m_flags.value_or(wxSizerFlags()));
		parent->SetSizerAndFit(box);
		return box;
	}

private:
	wxOrientation m_orient;
	std::string m_label;
	std::optional<wxSizerFlags> m_flags;
	std::tuple<W...> m_widgets;
};

template<CreateAndAddable... W>
struct HGroupBox : public GroupBox<W...>
{
	HGroupBox(W... widgets)
		: GroupBox<W...>(wxHORIZONTAL, "", widgets...)
	{
	}

	HGroupBox(wxSizerFlags flags, W... widgets)
		: GroupBox<W...>(wxHORIZONTAL, "", flags, widgets...)
	{
	}

	HGroupBox(const std::string& label, W... widgets)
		: GroupBox<W...>(wxHORIZONTAL, label, widgets...)
	{
	}

	HGroupBox(const std::string& label, wxSizerFlags flags, W... widgets)
		: GroupBox<W...>(wxHORIZONTAL, label, flags, widgets...)
	{
	}
};

template<CreateAndAddable... W>
struct VGroupBox : public GroupBox<W...>
{
	VGroupBox(W... widgets)
		: GroupBox<W...>(wxVERTICAL, "", widgets...)
	{
	}

	VGroupBox(wxSizerFlags flags, W... widgets)
		: GroupBox<W...>(wxVERTICAL, "", flags, widgets...)
	{
	}

	VGroupBox(const std::string& label, W... widgets)
		: GroupBox<W...>(wxVERTICAL, label, widgets...)
	{
	}

	VGroupBox(const std::string& label, wxSizerFlags flags, W... widgets)
		: GroupBox<W...>(wxVERTICAL, label, flags, widgets...)
	{
	}
};
