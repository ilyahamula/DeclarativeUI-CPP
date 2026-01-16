#pragma once

#include <wx/wx.h>
#include <tuple>

template <typename T>
concept CreateAndAddable = requires(T widget, wxWindow* parent, wxSizer* sizer, wxSizerFlags flags)
{
	widget.createAndAdd(parent, sizer, flags);
};

template <CreateAndAddable... W>
void createAndAdd(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, std::tuple<W...> widgets)
{
	std::apply([parent, sizer, flags](auto&&... widget) {
		(widget.createAndAdd(parent, sizer, flags), ...);
	}, widgets);
}

template <CreateAndAddable... W>
void createAndAdd(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags, W... widgets)
{
	createAndAdd(parent, sizer, flags, std::make_tuple(widgets...));
}