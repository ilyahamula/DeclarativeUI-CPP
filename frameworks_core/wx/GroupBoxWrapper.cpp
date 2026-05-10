#include "frameworks_core/GroupBoxWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <wx/statbox.h>

GroupBoxWrapper::GroupBoxWrapper(ControlWrapper* parent, Orientation orient, const std::string& label)
	: LayoutWrapper(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (orient == Orientation::Horizontal
		? "GroupBoxWrapper::GroupBoxWrapper() Horizontal\t-> new wxStaticBoxSizer()\n"
		: "GroupBoxWrapper::GroupBoxWrapper() Vertical\t-> new wxStaticBoxSizer()\n"));
#endif
	m_nativeSizer = new wxStaticBoxSizer(
		(orient == Orientation::Horizontal) ? wxHORIZONTAL : wxVERTICAL, parent->nativeHandle(), label);
	m_staticBox = ControlWrapper(static_cast<wxStaticBoxSizer*>(m_nativeSizer)->GetStaticBox());
}

ControlWrapper* GroupBoxWrapper::staticBox()
{
	return &m_staticBox;
}

void GroupBoxWrapper::finilizeLayout()
{
	LayoutWrapper::finilizeLayout();
}
