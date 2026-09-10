#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

struct LayoutNode;
class ILayoutBackend;

// The single source of layout truth: measure (constraints down, sizes up),
// SizeGroup reconcile, arrange (docs/specs/custom_layout_system/architecture.md).
// Backends only measure leaves and place rectangles.
//
// Border semantics: a node's Border() insets are its *margins*, kept outside
// its own `desired` size. The parent box adds the first child's leading and
// last child's trailing margin to its content extent, and collapses facing
// margins between siblings via max() — falling back to kDefaultGap when
// neither side sets one. The root node's margins are applied by run().
class LayoutEngine
{
public:
	// Inter-sibling spacing when neither adjacent Border is set (one
	// engine-owned constant, identical on every backend).
	static constexpr int kDefaultGap = 8;

	// Viewport a ScrollPanel falls back to on a scrolling axis when the caller
	// set no MaxSize there: the whole point of the panel is that its content
	// does NOT decide the window's size, so an uncapped axis needs a number.
	static constexpr int kDefaultScrollViewport = 240;

	// Width cap offered to an auto-fit root so wrapping content wraps
	// instead of widening the window without bound. Backends may override
	// via setMaxAutoFitWidth() with a fraction of the actual work area.
	static constexpr int kDefaultMaxAutoFitWidth = 1280;

	explicit LayoutEngine(ILayoutBackend& backend);

	// Constraints down, sizes up: fills node.desired for the whole subtree
	// and returns the root's desired size (margins of `node` excluded).
	Size measure(LayoutNode& node, const Constraints& c);

	// After measure: raises the desired size of nodes sharing a SizeGroup id
	// to the group maximum (per axis) and re-propagates the change upward —
	// every ancestor's desired is re-summed, so an auto-fit root reflects the
	// raised sizes. Runs to a fixpoint for nested/interacting groups.
	void reconcileSizeGroups(LayoutNode& root);

	// Top-down: fills node.frame for the whole subtree within `area` (the
	// node's own margins are already outside `area` — the caller spent them).
	// Main axis: measured extents, positive leftover to Proportion weights,
	// negative leftover per the shrink order (proportioned down to MinSize
	// floors first, then reverse declaration order, residue overflows and is
	// clipped by the backend). Cross axis: per crossAlign(kind, orientation).
	void arrange(LayoutNode& node, const Rect& area);

	// Measure + reconcile; returns the resolved content size. contentSize
	// {-1,-1} = auto-fit: measured under the max auto-fit width cap, the
	// result is the root's desired size plus its own margins. Split from
	// render() because ImGui needs the size before ImGui::Begin.
	Size resolve(LayoutNode& root, Size contentSize = { -1, -1 });

	// Arrange within the resolved content size (root margins inside it) and
	// traverse to the backend (beginContainer/place/endContainer). On ImGui
	// this must run inside the target window.
	void render(LayoutNode& root, Size resolvedSize);

	// Full pass: resolve + render. Returns the resolved content size.
	Size run(LayoutNode& root, Size contentSize = { -1, -1 });

	void setMaxAutoFitWidth(int width) { m_maxAutoFitWidth = width; }
	int maxAutoFitWidth() const { return m_maxAutoFitWidth; }

	// Main-axis gap between two adjacent siblings: max of the facing
	// margins, or kDefaultGap when both are zero.
	static int gapBetween(const LayoutNode& prev, const LayoutNode& next, Orientation orient);

private:
	static constexpr int kMaxReconcilePasses = 8;

	Size measureBox(LayoutNode& node, const Constraints& c);
	Size measureGrid(LayoutNode& node, const Constraints& c);
	Size measureScrollPanel(LayoutNode& node, const Constraints& c);
	Size measureSplitter(LayoutNode& node, const Constraints& c);
	Size measureTabPanel(LayoutNode& node, const Constraints& c);
	void arrangeBox(LayoutNode& node);
	void arrangeGrid(LayoutNode& node);
	void arrangeScrollPanel(LayoutNode& node);
	void arrangeSplitter(LayoutNode& node);
	void arrangeTabPanel(LayoutNode& node);
	void traverse(LayoutNode& node);

	ILayoutBackend& m_backend;
	int m_maxAutoFitWidth = kDefaultMaxAutoFitWidth;
};
