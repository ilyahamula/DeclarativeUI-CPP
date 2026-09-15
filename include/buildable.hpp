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

// A top-level window the engine lays a tree out into: Dialog and Window. It is
// what a node tree is shown IN, never a node in one -- which is why it is a
// concept of its own rather than a NodeBuildable.
template <typename T>
concept TopLevel = requires(T element) {
	{ element.show() } -> std::same_as<void>;
};

// A top-level window that can also be shown against a caller-owned flag, which
// is then the single truth about whether it is up: clearing it closes the
// window, closing the window clears it. Window today; Dialog in T3.6.
template <typename T>
concept FlagShowable = TopLevel<T> && requires(T element, bool& open) {
	{ element.show(open) } -> std::same_as<void>;
};
