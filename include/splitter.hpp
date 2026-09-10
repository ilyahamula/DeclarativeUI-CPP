#pragma once

#include <memory>
#include <optional>
#include <utility>

#include "buildable.hpp"
#include "frameworks_core/ControlWrappers.hpp"

// Two panes and a draggable sash between them.
//
//   HSplitter { tree, table }                       // side by side
//   VSplitter { editor, output }                    // stacked
//   HSplitter { dividerX, tree, table }             // bound to a caller's int
//
// The sash position is the FIRST pane's extent along the splitter's own axis,
// in pixels, and -1 (the default) means "half" -- resolved at arrange time,
// when the area is finally known. Bind it to an `int&` and a drag writes
// through to it, while writing it from anywhere else moves the sash; snapshot
// it from a literal and the splitter keeps its own copy.
//
// No backend contributes a native splitter here. wxSplitterWindow and QSplitter
// own their children's geometry, which is exactly what the layout engine exists
// to take back, so all three backends get the same engine-arranged panes and a
// 6 px leaf that knows how to be dragged. One tree, one set of frames.
//
// Neither pane can be dragged out of existence: `withMinPaneSize(a, b)` sets the
// floors (40 px each by default), and a pane's own MinSize raises its floor
// further -- whichever is larger wins.
template <NodeBuildable A, NodeBuildable B>
struct Splitter
{
	Splitter(Orientation orient, A first, B second)
		: m_orient(orient)
		, m_first(std::move(first))
		, m_second(std::move(second))
	{
	}

	Splitter(Orientation orient, LayoutFlags flags, A first, B second)
		: m_orient(orient)
		, m_flags(flags)
		, m_first(std::move(first))
		, m_second(std::move(second))
	{
	}

	// Snapshot the position as it stands now -- a literal lands here.
	Splitter(Orientation orient, const int& position, A first, B second)
		: m_orient(orient)
		, m_position(position)
		, m_first(std::move(first))
		, m_second(std::move(second))
	{
	}

	// Bind to a caller-owned int: dragging the sash writes through to it, and
	// rewriting it elsewhere moves the sash. A non-const lvalue selects this.
	Splitter(Orientation orient, int& position, A first, B second)
		: m_orient(orient)
		, m_position(position)
		, m_first(std::move(first))
		, m_second(std::move(second))
	{
	}

	Splitter(Orientation orient, LayoutFlags flags, const int& position, A first, B second)
		: m_orient(orient)
		, m_flags(flags)
		, m_position(position)
		, m_first(std::move(first))
		, m_second(std::move(second))
	{
	}

	Splitter(Orientation orient, LayoutFlags flags, int& position, A first, B second)
		: m_orient(orient)
		, m_flags(flags)
		, m_position(position)
		, m_first(std::move(first))
		, m_second(std::move(second))
	{
	}

	// Pixel floors for the two panes. A pane's own MinSize says the same thing
	// on the splitter's axis, and the larger of the two wins.
	Splitter& withMinPaneSize(int first, int second)
	{
		m_minFirst = first;
		m_minSecond = second;
		return *this;
	}

	// Disable the splitter and, with it, both panes and the sash. Snapshot the
	// flag as it stands now.
	Splitter& isDisabled(const bool& disabled = true)
	{
		m_disabled.snapshot(disabled);
		return *this;
	}

	// Bind to a caller-owned flag: flipping it disables/enables the whole
	// subtree without rebuilding the tree. A pane cannot opt back out --
	// disabling always cascades down.
	Splitter& isDisabled(bool& disabled)
	{
		m_disabled.bind(disabled);
		return *this;
	}

	std::unique_ptr<LayoutNode> buildNode()
	{
		auto node = makeSplitter(m_orient, m_flags.value_or(LayoutFlags{}));
		node->disabled = m_disabled;
		node->split.position = m_position;
		node->split.minFirst = m_minFirst;
		node->split.minSecond = m_minSecond;

		// Three children in order: pane, sash, pane. The sash is handed a
		// pointer INTO the node it hangs from -- the node tree outlives every
		// wrapper in it, so unlike a widget's own value this pointer is safe to
		// keep, and it is the one place the engine and the sash can meet.
		node->add(m_first.buildNode());
		node->add(makeLeaf(
			std::make_unique<SplitterSashWrapper>(&node->split,
				Position { -1, -1 }, sashSize(), 0L),
			// Expand(): a leaf would otherwise sit at its intrinsic cross size,
			// which for the sash is nothing at all.
			LayoutFlags().Expand()));
		node->add(m_second.buildNode());
		return node;
	}

private:
	// Thickness on the splitter's own axis; -1 leaves the other one to the
	// engine, which stretches the sash across both panes.
	Size sashSize() const
	{
		return m_orient == Orientation::Horizontal
			? Size { SplitterState::kSashThickness, -1 }
			: Size { -1, SplitterState::kSashThickness };
	}

	Orientation m_orient;
	std::optional<LayoutFlags> m_flags;
	BoundValue<int> m_position { -1 };
	int m_minFirst = SplitterState::kDefaultMinPane;
	int m_minSecond = SplitterState::kDefaultMinPane;
	DisabledFlag m_disabled;
	A m_first;
	B m_second;
};

// Panes side by side, sash vertical.
template <NodeBuildable A, NodeBuildable B>
struct HSplitter : public Splitter<A, B>
{
	HSplitter(A first, B second)
		: Splitter<A, B>(Orientation::Horizontal, std::move(first), std::move(second))
	{
	}

	HSplitter(LayoutFlags flags, A first, B second)
		: Splitter<A, B>(Orientation::Horizontal, flags, std::move(first), std::move(second))
	{
	}

	HSplitter(const int& position, A first, B second)
		: Splitter<A, B>(Orientation::Horizontal, position, std::move(first), std::move(second))
	{
	}

	HSplitter(int& position, A first, B second)
		: Splitter<A, B>(Orientation::Horizontal, position, std::move(first), std::move(second))
	{
	}

	HSplitter(LayoutFlags flags, const int& position, A first, B second)
		: Splitter<A, B>(Orientation::Horizontal, flags, position, std::move(first), std::move(second))
	{
	}

	HSplitter(LayoutFlags flags, int& position, A first, B second)
		: Splitter<A, B>(Orientation::Horizontal, flags, position, std::move(first), std::move(second))
	{
	}

	// re-declared so chaining keeps the HSplitter type
	HSplitter& withMinPaneSize(int first, int second)
	{
		Splitter<A, B>::withMinPaneSize(first, second);
		return *this;
	}

	HSplitter& isDisabled(const bool& disabled = true)
	{
		Splitter<A, B>::isDisabled(disabled);
		return *this;
	}

	HSplitter& isDisabled(bool& disabled)
	{
		Splitter<A, B>::isDisabled(disabled);
		return *this;
	}
};

// Panes stacked, sash horizontal.
template <NodeBuildable A, NodeBuildable B>
struct VSplitter : public Splitter<A, B>
{
	VSplitter(A first, B second)
		: Splitter<A, B>(Orientation::Vertical, std::move(first), std::move(second))
	{
	}

	VSplitter(LayoutFlags flags, A first, B second)
		: Splitter<A, B>(Orientation::Vertical, flags, std::move(first), std::move(second))
	{
	}

	VSplitter(const int& position, A first, B second)
		: Splitter<A, B>(Orientation::Vertical, position, std::move(first), std::move(second))
	{
	}

	VSplitter(int& position, A first, B second)
		: Splitter<A, B>(Orientation::Vertical, position, std::move(first), std::move(second))
	{
	}

	VSplitter(LayoutFlags flags, const int& position, A first, B second)
		: Splitter<A, B>(Orientation::Vertical, flags, position, std::move(first), std::move(second))
	{
	}

	VSplitter(LayoutFlags flags, int& position, A first, B second)
		: Splitter<A, B>(Orientation::Vertical, flags, position, std::move(first), std::move(second))
	{
	}

	// re-declared so chaining keeps the VSplitter type
	VSplitter& withMinPaneSize(int first, int second)
	{
		Splitter<A, B>::withMinPaneSize(first, second);
		return *this;
	}

	VSplitter& isDisabled(const bool& disabled = true)
	{
		Splitter<A, B>::isDisabled(disabled);
		return *this;
	}

	VSplitter& isDisabled(bool& disabled)
	{
		Splitter<A, B>::isDisabled(disabled);
		return *this;
	}
};
