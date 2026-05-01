#include "frameworks_core/TabPanelWrapper.hpp"

#ifdef USE_WX

TabPanelWrapper::TabPanelWrapper(ControlWrapper* parent)
{
	m_notebook = new wxNotebook(parent->nativeHandle(), wxID_ANY);
}

void TabPanelWrapper::addToLayout(LayoutWrapper* parentLayout, LayoutFlags flags)
{
	m_parentLayout = parentLayout;
	m_layoutFlags = flags;
}

void TabPanelWrapper::fitAsRoot(ControlWrapper* parent)
{
	m_rootParent = parent;
}

bool TabPanelWrapper::beginTab(const std::string& label)
{
	m_currentPanel = new wxPanel(m_notebook);
	m_currentLabel = label;
	m_panelWrapper = ControlWrapper(m_currentPanel);
	return true;
}

ControlWrapper* TabPanelWrapper::tabPageParent()
{
	return &m_panelWrapper;
}

void TabPanelWrapper::endTab()
{
	m_notebook->AddPage(m_currentPanel, m_currentLabel);
}

void TabPanelWrapper::finalize()
{
	if (m_parentLayout)
	{
		ControlWrapper notebookWrapper(m_notebook);
		m_parentLayout->add(&notebookWrapper, m_layoutFlags);
	}
	else if (m_rootParent)
	{
		auto* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(m_notebook, 1, wxEXPAND);
		LayoutWrapper wrapperLayout(sizer);
		m_rootParent->setLayout(&wrapperLayout);
	}
}

#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"
#include "frameworks_core/ImGuiHelpers/ImGuiWidgetIdManager.hpp"

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

#endif
