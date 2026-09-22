#pragma once

#include "frameworks_core/LayoutNode.hpp"

#include <concepts>
#include <memory>
#include <string>

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

// A single-line text field that can carry placeholder text. Declared per
// widget rather than on Widget<W>, because the modifier genuinely does not
// exist for a multi-line field: wxTextCtrl::SetHint does nothing on a
// wxTE_MULTILINE control on every wx port, so MultiLineTextCtrl would have to
// promise something one backend could not keep. A compile error is the honest
// answer, and this is the concept that states it.
template <typename T>
concept PlaceholderHost = requires(T element, std::string hint) {
	{ element.withPlaceholder(std::move(hint)) } -> std::same_as<T&>;
};

// A picture that can say what happens to its pixels inside the frame the
// engine gave it. Image alone: every other leaf either has no pixels of its
// own to scale (a label, a field) or hands them to a native control that
// decides for itself (a tool's icon), so the modifier lives on the one widget
// that can keep the promise on all three backends.
template <typename T>
concept ScaleModeHost = requires(T element) {
	{ element.withScaleMode(ScaleMode::Fit) } -> std::same_as<T&>;
};

// A label whose text can be aligned inside its frame. StaticText alone: a
// button, a check box or a field centres or left-aligns its own text by
// platform convention, and overriding that would look wrong rather than
// consistent.
template <typename T>
concept TextAlignHost = requires(T element) {
	{ element.withAlign(TextAlign::Right) } -> std::same_as<T&>;
};
