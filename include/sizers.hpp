#pragma once

#include <wx/wx.h>

#include <optional>
#include <tuple>

#include "create_and_add.hpp"

template<CreateAndAddable... W>
struct Sizer
{
	Sizer(wxOrientation orient, W... widgets)
		: m_orient(orient)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	Sizer(wxOrientation orient, wxSizerFlags flags, W... widgets)
		: m_orient(orient)
		, m_flags(flags)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	auto createAndAdd(wxWindow* parent, wxSizerFlags parentFlags)
	{
		auto* sizer = new wxBoxSizer(m_orient);
		::createAndAdd(parent, sizer, m_flags.value_or(parentFlags), m_widgets);
		return sizer;
	}

	auto createAndAdd(wxWindow* parent, wxSizer* parentSizer, wxSizerFlags parentFlags)
	{
		auto* sizer = createAndAdd(parent, parentFlags);
		parentSizer->Add(sizer, parentFlags);
		return sizer;
	}

	auto fitTo(wxWindow* parent)
	{
		auto* sizer = createAndAdd(parent, m_flags.value_or(wxSizerFlags()));
		parent->SetSizerAndFit(sizer);
		return sizer;
	}

private:
	wxOrientation m_orient;
	std::optional<wxSizerFlags> m_flags;
	std::tuple<W...> m_widgets;
};

template<CreateAndAddable... W>
struct HSizer : public Sizer<W...>
{
	HSizer(W... widgets)
		: Sizer<W...>(wxHORIZONTAL, widgets...)
	{
	}

	HSizer(wxSizerFlags flags, W... widgets)
		: Sizer<W...>(wxHORIZONTAL, flags, widgets...)
	{
	}
};

template<CreateAndAddable... W>
struct VSizer : public Sizer<W...>
{
	VSizer(W... widgets)
		: Sizer<W...>(wxVERTICAL, widgets...)
	{
	}

	VSizer(wxSizerFlags flags, W... widgets)
		: Sizer<W...>(wxVERTICAL, flags, widgets...)
	{
	}
};