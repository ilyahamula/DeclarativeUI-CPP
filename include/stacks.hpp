#pragma once

#include <wx/wx.h>

#include <optional>
#include <tuple>

#include "create_and_add.hpp"

template<CreateAndAddable... W>
struct Stack
{
	Stack(wxOrientation orient, W... widgets)
		: m_orient(orient)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	Stack(wxOrientation orient, wxSizerFlags flags, W... widgets)
		: m_orient(orient)
		, m_flags(flags)
		, m_widgets(std::make_tuple(widgets...))
	{
	}

	wxSizer* createAndAdd(wxWindow* parent, wxSizerFlags parentFlags)
	{
		auto sizer = new wxBoxSizer(m_orient);
		::createAndAdd(parent, sizer, m_flags.value_or(parentFlags), m_widgets);
		return sizer;
	}

	auto createAndAdd(wxWindow* parent, wxSizer* parentSizer, wxSizerFlags parentFlags)
	{
		auto sizer = createAndAdd(parent, parentFlags);
		parentSizer->Add(sizer, parentFlags);
		return sizer;
	}

	auto fitTo(wxWindow* parent)
	{
		auto sizer = createAndAdd(parent, m_flags.value_or(wxSizerFlags()));
		parent->SetSizerAndFit(sizer);
		return sizer;
	}

private:
	wxOrientation m_orient;
	std::optional<wxSizerFlags> m_flags;
	std::tuple<W...> m_widgets;
};

template<CreateAndAddable... W>
struct HStack : public Stack<W...>
{
	HStack(W... widgets)
		: Stack<W...>(wxHORIZONTAL, widgets...)
	{
	}

	HStack(wxSizerFlags flags, W... widgets)
		: Stack<W...>(wxHORIZONTAL, flags, widgets...)
	{
	}
};

template<CreateAndAddable... W>
struct VStack : public Stack<W...>
{
	VStack(W... widgets)
		: Stack<W...>(wxVERTICAL, widgets...)
	{
	}

	VStack(wxSizerFlags flags, W... widgets)
		: Stack<W...>(wxVERTICAL, flags, widgets...)
	{
	}
};
