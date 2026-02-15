#pragma once

#include "GeneralTypes.hpp"

#include <string>

#ifdef USE_WX
#include <wx/sizer.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

class LayoutFlags;
class ControlWrapper;

class LayoutWrapper
{
protected:
	Orientation m_orientation;
public:
	LayoutWrapper(Orientation orient);
	virtual ~LayoutWrapper();

	void add(LayoutWrapper* stack, LayoutFlags& flags);
	void add(ControlWrapper* widget, LayoutFlags& flags);

	virtual void finilizeLayout();

	static std::string indent();

#ifdef USE_WX
	explicit LayoutWrapper(wxSizer* sizer);
	wxSizer* nativeHandle() const;
protected:
	wxSizer* m_nativeSizer = nullptr;
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

private:
	static int s_depth;
};
