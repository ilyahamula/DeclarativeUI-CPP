#pragma once

#include "ControlWrapper.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>

struct LayoutNode;

class DialogWrapper : public ControlWrapper
{
public:
	DialogWrapper(const std::string& title, const Size& size);
	void show();

	// Size the window from the layout engine's result (auto-fit unless
	// `size` is explicit) and draw the already-built node tree. Not
	// user-resizable unless `resizable`; then the auto-fit size is the
	// initial and minimum window size. Takes ownership of the tree:
	// immediate backends drop it at end of call, retained backends keep it
	// alive with the window (for resize re-arrange). An engaged `position`
	// places the window's top-left corner where it opens; unset leaves the
	// placement to the platform.
	//
	// `modal` locks the rest of the application out for as long as the dialog
	// is up, WITHOUT blocking: this call returns immediately on every backend,
	// so the caller's event loop (or frame loop) keeps running and the dialog
	// reports what happened through its own callbacks rather than a return
	// value. wx holds a wxWindowDisabler, Qt sets Qt::ApplicationModal, and
	// ImGui draws the tree in a BeginPopupModal instead of a plain window.
	//
	// `open`, when non-null, is a caller-owned flag that is the single truth
	// about whether the dialog is up -- clearing it closes the dialog, closing
	// the dialog clears it. `onClose` fires exactly once either way. The
	// retained backends poll the flag the way they poll any other externally
	// written value (RefSync); ImGui hands it straight to ImGui::Begin or
	// BeginPopupModal. This is the same contract WindowWrapper carries, because
	// Dialog and Window are two spellings of one thing.
	static void runLayoutEngine(const std::string& title, const Size& size,
		std::unique_ptr<LayoutNode> root, bool resizable = false,
		const std::optional<Position>& position = std::nullopt,
		bool modal = false, std::function<void()> onClose = {}, bool* open = nullptr);
};
