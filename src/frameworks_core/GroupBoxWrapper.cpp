#include "frameworks_core/GroupBoxWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#ifdef USE_WX
#include <wx/statbox.h>

GroupBoxWrapper::GroupBoxWrapper(ControlWrapper* parent, Orientation orient, const std::string& label)
	: LayoutWrapper(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(orient == Orientation::Horizontal
		? "GroupBoxWrapper::GroupBoxWrapper() Horizontal -> new wxStaticBoxSizer()\n"
		: "GroupBoxWrapper::GroupBoxWrapper() Vertical -> new wxStaticBoxSizer()\n");
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
}
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"

GroupBoxWrapper::GroupBoxWrapper(ControlWrapper* parent, Orientation orient, const std::string& label)
	: LayoutWrapper(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(orient == Orientation::Horizontal
		? "GroupBoxWrapper::GroupBoxWrapper() Horizontal\n"
		: "GroupBoxWrapper::GroupBoxWrapper() Vertical\n");
#endif
	ImGui::BeginChild("##radio_frame", ImVec2(-FLT_MIN, -FLT_MIN), ImGuiChildFlags_Borders);
	if (!label.empty())
	{
		ImGui::Text("%s", label.c_str());
		ImGui::Separator();
	}
}

ControlWrapper* GroupBoxWrapper::staticBox()
{
	return &m_staticBox;
}

void GroupBoxWrapper::finilizeLayout()
{
#ifdef USE_LOGGER
	Logger::instance().log("GroupBoxWrapper::finilizeLayout() -> ImGui::EndChild()\n");
#endif
	ImGui::EndChild();
	LayoutWrapper::finilizeLayout();
}
#endif
