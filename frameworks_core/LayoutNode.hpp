#pragma once

#include "frameworks_core/ControlWrapper.hpp"
#include "frameworks_core/CoreTypes/GeneralTypes.hpp"
#include "frameworks_core/LayoutFlags.hpp"

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

	std::vector<std::unique_ptr<LayoutNode>> children;

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
		children.push_back(std::move(child));
		return *children.back();
	}

	bool isLeaf() const { return kind == NodeKind::Leaf; }
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
