#include "frameworks_core/TabPanelWrapper.hpp"
#include "imgui.h"
#include "frameworks_core/imgui/ImGuiWidgetIdManager.hpp"

TabPanelWrapper::TabPanelWrapper(ControlWrapper* parent)
	: m_parent(parent)
{
}

void TabPanelWrapper::addToLayout(LayoutWrapper* parentLayout, LayoutFlags flags)
{
	parentLayout->add(flags);
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	m_tabBarOpen = ImGui::BeginTabBar("##tabbar");
}

void TabPanelWrapper::fitAsRoot(ControlWrapper* parent)
{
	ImGui::PushID(WidgetIdManager::nextWidgetId());
	m_tabBarOpen = ImGui::BeginTabBar("##tabbar");
}

bool TabPanelWrapper::beginTab(const std::string& label)
{
	if (!m_tabBarOpen) return false;
	return ImGui::BeginTabItem(label.c_str());
}

ControlWrapper* TabPanelWrapper::tabPageParent()
{
	return m_parent;
}

void TabPanelWrapper::endTab()
{
	ImGui::EndTabItem();
}

void TabPanelWrapper::finalize()
{
	if (m_tabBarOpen)
		ImGui::EndTabBar();
	ImGui::PopID();
}
