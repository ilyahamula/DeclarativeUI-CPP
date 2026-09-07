#pragma once

#include "frameworks_core/ControlWrapper.hpp"
#include "frameworks_core/CoreTypes/GeneralTypes.hpp"
#include "frameworks_core/LayoutFlags.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Backend-agnostic mirror of the declarative tree, plus storage for the
// engine's results (docs/specs/custom_layout_system/architecture.md).
// Built by the Stack/GroupBox/Dialog/TabPanel templates; consumed by
// LayoutEngine (measure fills `desired`, arrange fills `frame`).
struct LayoutNode
{
	NodeKind kind = NodeKind::Box;
	Orientation orientation = Orientation::Vertical; // for Box/GroupBox
	LayoutFlags flags;                               // how THIS node sits in its parent
	std::string label;                               // GroupBox title / Tab title
	int columns = 0;                                 // Grid only: cells fill rows
	                                                 // left-to-right, so a cell's
	                                                 // column is index % columns and
	                                                 // its row index / columns

	std::vector<std::unique_ptr<LayoutNode>> children;
	const LayoutNode* parent = nullptr;              // set by add(); disabling walks it

	// Container-level disable, inherited by the whole subtree. Set from the
	// Stack/GroupBox/TabPanel/Tab templates; leaves keep their own flag in
	// their wrapper. Disabling cascades DOWN and cannot be opted out of, which
	// is how wx, Qt and ImGui::BeginDisabled all behave natively.
	DisabledFlag disabled;

	ControlWrapper* widget = nullptr;                // set for Leaf nodes
	std::unique_ptr<ControlWrapper> ownedWidget;     // set when the tree owns the wrapper
	                                                 // (engine path: wrappers must
	                                                 // outlive tree build until place)

	// engine outputs (filled per layout pass)
	Size desired { 0, 0 };                           // from measure()
	Rect frame { 0, 0, 0, 0 };                       // absolute, from arrange()
	EdgeInsets chrome;                               // container chrome, from backend

	// Appends a child and returns a reference to it (the tree keeps ownership).
	LayoutNode& add(std::unique_ptr<LayoutNode> child)
	{
		child->parent = this;
		children.push_back(std::move(child));
		return *children.back();
	}

	bool isLeaf() const { return kind == NodeKind::Leaf; }

	// The state a backend must apply right now: this node's own flag, every
	// ancestor container's, and -- for a leaf -- its widget's. Recomputed per
	// pass rather than baked at build time, because any of those may be bound
	// to a caller-owned flag that has since flipped.
	bool isDisabledEffective() const
	{
		if (widget != nullptr && widget->isDisabled())
			return true;
		for (const LayoutNode* node = this; node != nullptr; node = node->parent)
		{
			if (node->disabled.get())
				return true;
		}
		return false;
	}

	// What a retained backend has to keep watching to stay in step, gathered
	// once when the native control is created: the caller-owned flags in the
	// chain, plus whether some snapshot in it already settled the answer.
	// Both empty and !fixed means the state can never change again.
	struct DisabledSources
	{
		std::vector<const bool*> refs;
		bool fixed = false;
	};

	DisabledSources disabledSources() const
	{
		DisabledSources sources;
		// The bools are caller-owned and outlive every window, so a backend may
		// capture them; the nodes and wrappers must never be captured.
		auto collect = [&sources](const DisabledFlag& flag) {
			if (const bool* bound = flag.boundValue())
				sources.refs.push_back(bound);
			else if (flag.get())
				sources.fixed = true;
		};
		if (widget != nullptr)
			collect(widget->disabledFlag());
		for (const LayoutNode* node = this; node != nullptr; node = node->parent)
			collect(node->disabled);
		return sources;
	}
};

inline std::unique_ptr<LayoutNode> makeBox(Orientation orient, LayoutFlags flags = {})
{
	auto node = std::make_unique<LayoutNode>();
	node->kind = NodeKind::Box;
	node->orientation = orient;
	node->flags = flags;
	return node;
}

inline std::unique_ptr<LayoutNode> makeGroupBox(Orientation orient, std::string label, LayoutFlags flags = {})
{
	auto node = std::make_unique<LayoutNode>();
	node->kind = NodeKind::GroupBox;
	node->orientation = orient;
	node->label = std::move(label);
	node->flags = flags;
	return node;
}

// A grid has no chrome and no backend scope -- it is a Box that happens to
// arrange its children in `columns` columns, so every backend treats it as one.
inline std::unique_ptr<LayoutNode> makeGrid(int columns, LayoutFlags flags = {})
{
	auto node = std::make_unique<LayoutNode>();
	node->kind = NodeKind::Grid;
	node->columns = std::max(1, columns);
	node->flags = flags;
	return node;
}

inline std::unique_ptr<LayoutNode> makeTabPanel(LayoutFlags flags = {})
{
	auto node = std::make_unique<LayoutNode>();
	node->kind = NodeKind::TabPanel;
	node->flags = flags;
	return node;
}

inline std::unique_ptr<LayoutNode> makeLeaf(ControlWrapper* widget, LayoutFlags flags = {})
{
	auto node = std::make_unique<LayoutNode>();
	node->kind = NodeKind::Leaf;
	node->widget = widget;
	node->flags = flags;
	return node;
}

inline std::unique_ptr<LayoutNode> makeLeaf(std::unique_ptr<ControlWrapper> widget, LayoutFlags flags = {})
{
	auto node = makeLeaf(widget.get(), flags);
	node->ownedWidget = std::move(widget);
	return node;
}
