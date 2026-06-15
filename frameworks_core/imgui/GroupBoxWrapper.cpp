#include "frameworks_core/GroupBoxWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include "imgui.h"
#include "frameworks_core/imgui/ImGuiWidgetIdManager.hpp"

GroupBoxWrapper::GroupBoxWrapper(ControlWrapper* parent, Orientation orient, const std::string& label)
	: LayoutWrapper(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (orient == Orientation::Horizontal
		? "GroupBoxWrapper::GroupBoxWrapper() Horizontal\t-> ImGui::BeginChild()\n"
		: "GroupBoxWrapper::GroupBoxWrapper() Vertical\t-> ImGui::BeginChild()\n"));
#endif
	std::string childId = "##groupbox_" + std::to_string(WidgetIdManager::nextGroupBoxId());
	ImGui::BeginChild(childId.c_str(), ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
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
	saveCenterVCache();
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "GroupBoxWrapper::finilizeLayout() Horizontal\t-> ImGui::EndChild()\n"
		: "GroupBoxWrapper::finilizeLayout() Vertical\t-> ImGui::EndChild()\n"));
#endif
	ImGui::EndChild();
	LayoutWrapper::finilizeLayout();
}
