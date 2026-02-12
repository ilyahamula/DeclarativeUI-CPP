#pragma once

#include "GeneralTypes.hpp"

#ifdef USE_WX
#include <wx/sizer.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

class LayoutFlags;

class LayoutWrapper
{
	Orientation m_orientation;
public:
	LayoutWrapper(Orientation orient);
	virtual ~LayoutWrapper();
	void add(LayoutWrapper* stack, LayoutFlags& flags);

#ifdef USE_WX
	explicit LayoutWrapper(wxSizer* sizer);
	wxSizer* nativeHandle() const;
protected:
	wxSizer* m_nativeSizer = nullptr;
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
};
