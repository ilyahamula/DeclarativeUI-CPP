#include "frameworks_core/LayoutWrapper.hpp"
#include "frameworks_core/LayoutFlags.hpp"

LayoutWrapper::LayoutWrapper(Orientation orient)
	: m_orientation(orient)
{
#ifdef USE_WX
	m_nativeSizer = new wxBoxSizer(orient == Orientation::Horizontal ? wxHORIZONTAL : wxVERTICAL);
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
}

LayoutWrapper::~LayoutWrapper() = default;

#ifdef USE_WX
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
#endif
