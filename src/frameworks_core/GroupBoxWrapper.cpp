#include "frameworks_core/GroupBoxWrapper.hpp"

#include <wx/statbox.h>

GroupBoxWrapper::GroupBoxWrapper(ControlWrapper* parent, Orientation orient, const std::string& label)
	: LayoutWrapper(new wxStaticBoxSizer(
		(orient == Orientation::Horizontal) ? wxHORIZONTAL : wxVERTICAL,
		parent->nativeHandle(), label))
	, m_staticBox(static_cast<wxStaticBoxSizer*>(m_nativeSizer)->GetStaticBox())
{

}

ControlWrapper* GroupBoxWrapper::staticBox()
{
	return &m_staticBox;
}
