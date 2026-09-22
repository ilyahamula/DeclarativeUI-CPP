#pragma once

#include "frameworks_core/LayoutNode.hpp"

#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <utility>

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
// window, closing the window clears it. Both spellings carry it -- a lifecycle
// is not something a Dialog and a Window should disagree about.
template <typename T>
concept FlagShowable = TopLevel<T> && requires(T element, bool& open) {
	{ element.show(open) } -> std::same_as<void>;
};

// A top-level window that reports its own closing, exactly once, whichever side
// closed it. Paired with FlagShowable: the flag says whether the window is up,
// the callback says when that changed.
template <typename T>
concept CloseObservable = TopLevel<T> && requires(T element, std::function<void()> callback) {
	{ element.onClose(std::move(callback)) } -> std::same_as<T&>;
};

// A top-level window that can lock the rest of the application out while it is
// up, without blocking the caller. Dialog alone: a Window is the application
// frame, and a frame that refuses input to every other window is a dialog by
// another name -- so the modifier lives on the one spelling whose role it fits,
// and asking a Window for it must not compile.
template <typename T>
concept ModalWindow = TopLevel<T> && requires(T element) {
	{ element.Modal() } -> std::same_as<T&>;
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
