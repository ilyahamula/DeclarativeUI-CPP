#pragma once

#include "ControlWrapper.hpp"

#include <memory>
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
	// alive with the window (for resize re-arrange).
	static void runLayoutEngine(const std::string& title, const Size& size,
		std::unique_ptr<LayoutNode> root, bool resizable = false);
};
