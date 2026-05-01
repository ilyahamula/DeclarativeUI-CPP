#pragma once

#include <string>
#include <tuple>

#include "create_and_add.hpp"
#include "fittable_layout.hpp"
#include "frameworks_core/TabPanelWrapper.hpp"

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

	void createAndAdd(ControlWrapper* parent, LayoutWrapper* parentLayout, LayoutFlags flags)
	{
		TabPanelWrapper wrapper(parent);
		wrapper.addToLayout(parentLayout, flags);
		addTabs(wrapper);
		wrapper.finalize();
	}

	void fitTo(ControlWrapper* parent)
	{
		TabPanelWrapper wrapper(parent);
		wrapper.fitAsRoot(parent);
		addTabs(wrapper);
		wrapper.finalize();
	}

private:
	void addTabs(TabPanelWrapper& wrapper)
	{
		std::apply([&](auto&... tab) {
			([&] {
				if (wrapper.beginTab(tab.m_label))
				{
					tab.m_content.fitTo(wrapper.tabPageParent());
					wrapper.endTab();
				}
			}(), ...);
		}, m_tabs);
	}
};
