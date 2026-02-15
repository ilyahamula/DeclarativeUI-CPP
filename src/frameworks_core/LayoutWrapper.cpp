#include "frameworks_core/LayoutWrapper.hpp"
#include "frameworks_core/LayoutFlags.hpp"

LayoutWrapper::~LayoutWrapper() = default;

#ifdef USE_WX

LayoutWrapper::LayoutWrapper(Orientation orient)
	: m_orientation(orient)
{
	m_nativeSizer = new wxBoxSizer(orient == Orientation::Horizontal ? wxHORIZONTAL : wxVERTICAL);
}

LayoutWrapper::LayoutWrapper(wxSizer* sizer)
	: m_nativeSizer(sizer)
{
}

void LayoutWrapper::add(LayoutWrapper* stack, LayoutFlags& flags)
{
	m_nativeSizer->Add(stack->nativeHandle(), flags.wx());
}

wxSizer* LayoutWrapper::nativeHandle() const
{
	return m_nativeSizer;
}
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"

LayoutWrapper::LayoutWrapper(Orientation orient)
	: m_orientation(orient)
{
}

void LayoutWrapper::add(LayoutWrapper* stack, LayoutFlags& flags)
{
}
#endif
