#pragma once

#include "frameworks_core/LayoutNode.hpp"

#include <concepts>
#include <memory>

// A declarative element that can emit its layout node (widgets, stacks,
// group boxes, tab panels). The node tree is the only path into the layout
// engine (docs/specs/custom_layout_system/architecture.md).
template <typename T>
concept NodeBuildable = requires(T element) {
	{ element.buildNode() } -> std::same_as<std::unique_ptr<LayoutNode>>;
};
