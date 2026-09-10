#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "buildable.hpp"
#include "frameworks_core/ControlWrappers.hpp"

// A collapsible section: a clickable header with a content tree that folds away
// behind it, so advanced options need not cost the dialog any room until they
// are asked for.
//
//   Expander { "Advanced", VStack { ... } }                  // starts closed
//   Expander { "Advanced", true, VStack { ... } }            // starts open
//   Expander { "Advanced", showAdvanced, VStack { ... } }    // bound to a bool&
//
// Collapsed, the content contributes nothing at all -- not its size, not its
// margins, not the gap that would sit above it -- so the section measures
// exactly its header and an auto-fit dialog shrinks to match. It is the same
// bargain a tab page makes: the subtree stays in the tree, and the backend is
// told not to show it.
//
// Bind the open state to a `bool&` and clicking the header writes through to it,
// while writing it from anywhere else opens or closes the section; snapshot it
// from a literal and the expander keeps its own copy.
//
// Flags set on the content element apply to it inside the expander's content
// box. Flags for the section as a whole -- how it sits in ITS parent -- belong
// in the `LayoutFlags` overloads below.
template <NodeBuildable Content>
struct Expander
{
	Expander(const std::string& title, Content content)
		: m_title(title)
		, m_content(std::move(content))
	{
	}

	Expander(const std::string& title, LayoutFlags flags, Content content)
		: m_title(title)
		, m_flags(flags)
		, m_content(std::move(content))
	{
	}

	// Snapshot the open state as it stands now -- a literal lands here.
	Expander(const std::string& title, const bool& expanded, Content content)
		: m_title(title)
		, m_expanded(expanded)
		, m_content(std::move(content))
	{
	}

	// Bind to a caller-owned flag: clicking the header writes through to it, and
	// rewriting it elsewhere opens or closes the section. A non-const lvalue
	// selects this.
	Expander(const std::string& title, bool& expanded, Content content)
		: m_title(title)
		, m_expanded(expanded)
		, m_content(std::move(content))
	{
	}

	Expander(const std::string& title, LayoutFlags flags, const bool& expanded, Content content)
		: m_title(title)
		, m_flags(flags)
		, m_expanded(expanded)
		, m_content(std::move(content))
	{
	}

	Expander(const std::string& title, LayoutFlags flags, bool& expanded, Content content)
		: m_title(title)
		, m_flags(flags)
		, m_expanded(expanded)
		, m_content(std::move(content))
	{
	}

	// Disable the section and, with it, the header and everything inside it.
	// Snapshot the flag as it stands now.
	Expander& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
		return *this;
	}

	// Bind to a caller-owned flag: flipping it disables/enables the whole
	// subtree without rebuilding the tree. The content cannot opt back out --
	// disabling always cascades down.
	Expander& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeExpander(m_title, m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		node->expander.expanded = m_expanded;

		// Two children in order: header, content. The header is handed a
		// pointer INTO the node it hangs from -- the node tree outlives every
		// wrapper in it, so unlike a widget's own value this pointer is safe to
		// keep, and it is the one place the engine and the header can meet.
		node->add(makeLeaf(
			std::make_unique<ExpanderHeaderWrapper>(m_title, &node->expander,
				Position { -1, -1 }, Size { -1, -1 }, 0L),
			// Expand(): a leaf would otherwise sit at its intrinsic width, and
			// a title row that does not span the section is a title row only
			// half of which can be clicked.
			LayoutFlags().Expand()));

		// The content always hangs from a container, even when the caller
		// passed a single leaf: it is the scope the retained backends hide in
		// one call, and only a container gets a beginContainer() to report
		// itself invisible from.
		auto& box = node->add(makeBox(Orientation::Vertical));
		box.add(m_content.buildNode());
		return node;
	}

private:
	std::string m_title;
	std::optional<LayoutFlags> m_flags;
	BoundValue<bool> m_expanded { false };
	DisabledFlag m_disabled;
	Content m_content;
};

template <NodeBuildable Content>
Expander(const std::string&, Content) -> Expander<Content>;

template <NodeBuildable Content>
Expander(const std::string&, LayoutFlags, Content) -> Expander<Content>;

template <NodeBuildable Content>
Expander(const std::string&, const bool&, Content) -> Expander<Content>;

template <NodeBuildable Content>
Expander(const std::string&, bool&, Content) -> Expander<Content>;

template <NodeBuildable Content>
Expander(const std::string&, LayoutFlags, const bool&, Content) -> Expander<Content>;

template <NodeBuildable Content>
Expander(const std::string&, LayoutFlags, bool&, Content) -> Expander<Content>;
