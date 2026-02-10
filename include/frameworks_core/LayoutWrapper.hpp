#pragma once

#include "GeneralTypes.hpp"

#include <wx/sizer.h>

class LayoutFlags;

class LayoutWrapper
{
	Orientation m_orientation;
public:
	LayoutWrapper(Orientation orient);
	virtual ~LayoutWrapper();
	void add(LayoutWrapper* stack, LayoutFlags& flags);

	explicit LayoutWrapper(wxSizer* sizer);
	wxSizer* nativeHandle() const;
protected:
	wxSizer* m_nativeSizer = nullptr;
};
