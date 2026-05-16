#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

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
	Tab(const std::string& label, W content)
		: m_label(label)
		, m_content(std::move(content))
	{}

	const std::string& label() const { return m_label; }
	W& content() { return m_content; }

private:
	std::string m_label;
	W m_content;
};

template<TabContent W>
Tab(const std::string&, W) -> Tab<W>;

// ---------------------------------------------------------------------------
// IsTab concept
// ---------------------------------------------------------------------------
template<typename T>
struct is_tab : std::false_type {};

template<TabContent W>
struct is_tab<Tab<W>> : std::true_type {};

template<typename T>
concept IsTab = is_tab<T>::value;

// ---------------------------------------------------------------------------
// TabPanel — holds a set of Tab<W> instances
// ---------------------------------------------------------------------------
template<IsTab... Tabs>
struct TabPanel
{
	explicit TabPanel(Tabs... tabs)
		: m_tabs(std::make_tuple(std::move(tabs)...))
	{}

	TabPanel(LayoutFlags flags, Tabs... tabs)
		: m_flags(flags)
		, m_tabs(std::make_tuple(std::move(tabs)...))
	{}

	void createAndAdd(ControlWrapper* parent, LayoutWrapper* parentLayout, LayoutFlags flags)
	{
		TabPanelWrapper wrapper(parent);
		wrapper.addToLayout(parentLayout, m_flags.value_or(flags));
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
	std::optional<LayoutFlags> m_flags;
	std::tuple<Tabs...> m_tabs;

	void addTabs(TabPanelWrapper& wrapper)
	{
		std::apply([&](auto&... tab) {
			([&] {
				if (wrapper.beginTab(tab.label()))
				{
					tab.content().fitTo(wrapper.tabPageParent());
					wrapper.endTab();
				}
			}(), ...);
		}, m_tabs);
	}
};
