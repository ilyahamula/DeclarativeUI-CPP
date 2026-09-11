#pragma once

#include <memory>
#include <optional>
#include <utility>

#include "buildable.hpp"

// A scrollable viewport around one content element, so a long list does not
// force the dialog past the screen.
//
// The panel's own size on a scrolling axis is deliberately NOT its content's:
// it is the smaller of the content, the `MaxSize` the caller set on that axis,
// and `LayoutEngine::kDefaultScrollViewport`. On a non-scrolling axis it sizes
// to its content exactly like a `Box`, and the content fills the viewport there
// however short it is.
//
// The content is then laid out in the panel's own space at its full desired
// extent -- a virtual rect the backend scrolls over. Everything inside is
// therefore positioned relative to the panel, the same way a tab page's
// children are.
//
//   ScrollPanel { VStack { ...many rows... } }
//       .withScroll(ScrollAxis::Vertical)
//       .withFlags(LayoutFlags().Expand().MaxSize({-1, 240}))
//
// Scrolling is vertical unless `withScroll()` says otherwise.
template <NodeBuildable Content>
struct ScrollPanel
{
	explicit ScrollPanel(Content content)
		: m_content(std::move(content))
	{
	}

	ScrollPanel(LayoutFlags flags, Content content)
		: m_flags(flags)
		, m_content(std::move(content))
	{
	}

	// Which axes scroll. `Both` caps and scrolls each axis independently.
	ScrollPanel& withScroll(ScrollAxis axis)
	{
		m_scroll = axis;
		return *this;
	}

	// Disable the panel and, with it, everything scrolled inside it. Snapshot
	// the flag as it stands now.
	ScrollPanel& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
		return *this;
	}

	// Bind to a caller-owned flag: flipping it disables/enables the whole
	// subtree without rebuilding the tree. Content cannot opt back out --
	// disabling always cascades down.
	ScrollPanel& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeScrollPanel(m_scroll, m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		node->add(m_content.buildNode());
		return node;
	}

private:
	ScrollAxis m_scroll = ScrollAxis::Vertical;
	std::optional<LayoutFlags> m_flags;
	DisabledFlag m_disabled;
	Content m_content;
};
