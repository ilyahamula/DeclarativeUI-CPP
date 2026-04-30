#pragma once

#include <string>
#include <tuple>

#include "create_and_add.hpp"
#include "fittable_layout.hpp"
#include "frameworks_core/LayoutWrapper.hpp"
#include "frameworks_core/ControlWrapper.hpp"
#include "frameworks_core/LayoutFlags.hpp"

#ifdef USE_WX
#include <wx/notebook.h>
#include <wx/panel.h>
#elif defined(USE_IMGUI)
#include "imgui.h"
#include "frameworks_core/ImGuiWidgetIdManager.hpp"
#endif

template<typename T>
concept TabContent = CreateAndAddable<T> && FittableLayout<T>;

// ---------------------------------------------------------------------------
// Tab — a single named tab with a content widget tree
// ---------------------------------------------------------------------------
template<TabContent W>
struct Tab
{
	std::string m_label;
	W m_content;

	Tab(const std::string& label, W content)
		: m_label(label)
		, m_content(std::move(content))
	{}
};

template<TabContent W>
Tab(const std::string&, W) -> Tab<W>;

// ---------------------------------------------------------------------------
// TabPanel — holds a set of Tab<W> instances
// ---------------------------------------------------------------------------
template<typename... Tabs>
struct TabPanel
{
	std::tuple<Tabs...> m_tabs;

	explicit TabPanel(Tabs... tabs)
		: m_tabs(std::make_tuple(std::move(tabs)...))
	{}

	// Used when TabPanel is a child of VStack / HStack
	void createAndAdd(ControlWrapper* parent, LayoutWrapper* parentLayout, LayoutFlags flags)
	{
#ifdef USE_WX
		auto* notebook = makeNotebook(parent);
		ControlWrapper notebookWrapper(notebook);
		parentLayout->add(&notebookWrapper, flags);
#elif defined(USE_IMGUI)
		parentLayout->add(flags);
		renderTabBar(parent);
#endif
	}

	// Used when TabPanel is the direct content of Dialog
	void fitTo(ControlWrapper* parent)
	{
#ifdef USE_WX
		auto* notebook = makeNotebook(parent);
		auto* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(notebook, 1, wxEXPAND);
		LayoutWrapper wrapperLayout(sizer);
		parent->setLayout(&wrapperLayout);
#elif defined(USE_IMGUI)
		renderTabBar(parent);
#endif
	}

private:
#ifdef USE_WX
	wxNotebook* makeNotebook(ControlWrapper* parent)
	{
		auto* notebook = new wxNotebook(parent->nativeHandle(), wxID_ANY);
		std::apply([&](auto&... tab) {
			([&] {
				auto* panel = new wxPanel(notebook);
				ControlWrapper panelWrapper(panel);
				tab.m_content.fitTo(&panelWrapper);
				notebook->AddPage(panel, tab.m_label);
			}(), ...);
		}, m_tabs);
		return notebook;
	}
#elif defined(USE_IMGUI)
	void renderTabBar(ControlWrapper* parent)
	{
		ImGui::PushID(WidgetIdManager::nextWidgetId());
		if (ImGui::BeginTabBar("##tabbar"))
		{
			std::apply([&](auto&... tab) {
				([&] {
					if (ImGui::BeginTabItem(tab.m_label.c_str()))
					{
						LayoutWrapper contentLayout(Orientation::Vertical);
						tab.m_content.createAndAdd(parent, &contentLayout, LayoutFlags());
						contentLayout.finilizeLayout();
						ImGui::EndTabItem();
					}
				}(), ...);
			}, m_tabs);
			ImGui::EndTabBar();
		}
		ImGui::PopID();
	}
#endif
};
