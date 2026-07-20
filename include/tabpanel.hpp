#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

#include "buildable.hpp"

template<typename T>
concept TabContent = NodeBuildable<T>;

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

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeTabPanel(m_flags.value_or(LayoutFlags{}));
		std::apply([&](auto&... tab) {
			([&] {
				auto& page = node->add(tab.content().buildNode());
				page.label = tab.label();
			}(), ...);
		}, m_tabs);
		return node;
	}

private:
	std::optional<LayoutFlags> m_flags;
	std::tuple<Tabs...> m_tabs;
};
