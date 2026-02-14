#include "frameworks_core/GroupBoxWrapper.hpp"

#ifdef USE_WX
#include <wx/statbox.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

GroupBoxWrapper::GroupBoxWrapper(ControlWrapper* parent, Orientation orient, const std::string& label)
#ifdef USE_WX
	: LayoutWrapper(new wxStaticBoxSizer(
		(orient == Orientation::Horizontal) ? wxHORIZONTAL : wxVERTICAL,
		parent->nativeHandle(), label))
#elif defined(USE_QT)
	: LayoutWrapper(orient)
#elif defined(USE_IMGUI)
	: LayoutWrapper(orient)
#else
	: LayoutWrapper(orient)
#endif
{
#ifdef USE_WX
	m_staticBox = ControlWrapper(static_cast<wxStaticBoxSizer*>(m_nativeSizer)->GetStaticBox());
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
}

ControlWrapper* GroupBoxWrapper::staticBox()
{
	return &m_staticBox;
}
