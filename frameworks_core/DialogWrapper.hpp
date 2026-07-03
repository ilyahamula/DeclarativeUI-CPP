#pragma once

#include "ControlWrapper.hpp"

#include <string>

struct LayoutNode;

class DialogWrapper : public ControlWrapper
{
public:
	DialogWrapper(const std::string& title, const Size& size);
	void show();

#ifdef USE_LAYOUT_ENGINE
	// Engine path: size the window from the layout engine's result
	// (non-resizable; auto-fit unless `size` is explicit) and draw the
	// already-built node tree.
	static void runLayoutEngine(const std::string& title, const Size& size, LayoutNode& root);
#endif
};
