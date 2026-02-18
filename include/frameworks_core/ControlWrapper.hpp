#pragma once

#include "GeneralTypes.hpp"
#include "LayoutFlags.hpp"
#include "LayoutWrapper.hpp"

#ifdef USE_WX
#include <wx/window.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

class ControlWrapper
{
public:
	ControlWrapper();
	virtual ~ControlWrapper();

	virtual void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags);
	void setLayout(LayoutWrapper* layout);

#ifdef USE_WX
	ControlWrapper(wxWindow* nativeWidget);
	wxWindow* nativeHandle() const;

protected:
	wxWindow* m_nativeWidget = nullptr;
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
};
