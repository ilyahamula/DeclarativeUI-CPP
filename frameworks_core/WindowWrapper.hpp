#pragma once

#include "CoreTypes/GeneralTypes.hpp"

#include <functional>
#include <memory>
#include <string>

struct LayoutNode;

// Backend entry point for Window (include/window.hpp).
//
// Unlike the *Wrapper classes this is NOT a ControlWrapper: a top-level window
// is never a leaf in the node tree, it is what the tree is laid out INTO. It
// owns no data either -- everything it needs arrives as arguments -- so it is a
// namespace with a backend-specific body rather than an object.
class WindowWrapper
{
public:
	// Size the frame from the layout engine's result (auto-fit unless `size` is
	// explicit) and lay the already-built node tree out inside its client area.
	// `resizable` keeps the auto-fit size as the minimum, so content can never
	// be shrunk into clipping. Takes ownership of the tree: retained backends
	// keep it alive with the window, immediate ones drop it at end of call.
	//
	// `open`, when non-null, is a caller-owned flag that is the single truth
	// about whether the window is up -- clearing it closes the window, closing
	// the window clears it. `onClose` fires exactly once either way. The retained
	// backends poll the flag the way they poll any other externally-written value
	// (RefSync); ImGui hands it straight to ImGui::Begin.
	static void runLayoutEngine(const std::string& title, const Size& size,
		std::unique_ptr<LayoutNode> root, bool resizable = true,
		std::function<void()> onClose = {}, bool* open = nullptr);
};
