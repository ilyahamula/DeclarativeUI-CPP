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
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"

GroupBoxWrapper::GroupBoxWrapper(ControlWrapper* parent, Orientation orient, const std::string& label)
	: LayoutWrapper(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (orient == Orientation::Horizontal
		? "GroupBoxWrapper::GroupBoxWrapper() Horizontal\t-> ImGui::BeginChild()\n"
		: "GroupBoxWrapper::GroupBoxWrapper() Vertical\t-> ImGui::BeginChild()\n"));
#endif
	std::string childId = "##groupbox_" + std::to_string(s_groupBoxId++);
	ImGui::BeginChild(childId.c_str(), ImVec2(-FLT_MIN, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
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
	// Save height cache BEFORE EndChild so GetItemRectSize() measures the last child item
	saveCenterVCache();
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "GroupBoxWrapper::finilizeLayout() Horizontal\t-> ImGui::EndChild()\n"
		: "GroupBoxWrapper::finilizeLayout() Vertical\t-> ImGui::EndChild()\n"));
#endif
	ImGui::EndChild();
	LayoutWrapper::finilizeLayout();
}
#endif
