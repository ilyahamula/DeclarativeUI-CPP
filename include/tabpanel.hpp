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

	// Disable this page: everything on it greys out. The tab label itself stays
	// selectable -- wxNotebook has no per-tab enable, so content-only is the
	// one behavior all three backends share; disable the whole TabPanel to lock
	// tab switching. Snapshot / caller-owned-flag overloads as on the widgets.
	Tab& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
		return *this;
	}

	Tab& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	const DisabledFlag& disabledFlag() const { return m_disabled; }

private:
	std::string m_label;
	DisabledFlag m_disabled;
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

	// Disable the panel and, with it, every page and every widget on them.
	// Snapshot / caller-owned-flag overloads exactly as on the widgets.
	TabPanel& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
		return *this;
	}

	TabPanel& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeTabPanel(m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		std::apply([&](auto&... tab) {
			([&] {
				auto& page = node->add(tab.content().buildNode());
				page.label = tab.label();
				page.disabled = tab.disabledFlag();
			}(), ...);
		}, m_tabs);
		return node;
	}

private:
	std::optional<LayoutFlags> m_flags;
	DisabledFlag m_disabled;
	std::tuple<Tabs...> m_tabs;
};
