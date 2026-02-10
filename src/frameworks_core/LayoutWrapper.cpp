#include "frameworks_core/LayoutWrapper.hpp"
#include "frameworks_core/LayoutFlags.hpp"

LayoutWrapper::LayoutWrapper(Orientation orient)
	: m_orientation(orient)
{
	m_nativeSizer = new wxBoxSizer(orient == Orientation::Horizontal ? wxHORIZONTAL : wxVERTICAL);
}

LayoutWrapper::LayoutWrapper(wxSizer* sizer)
	: m_nativeSizer(sizer)
{
}

LayoutWrapper::~LayoutWrapper() = default;

void LayoutWrapper::add(LayoutWrapper* stack, LayoutFlags& flags)
{
	m_nativeSizer->Add(stack->nativeHandle(), flags.wx());
}

wxSizer* LayoutWrapper::nativeHandle() const
{
	return m_nativeSizer;
}
