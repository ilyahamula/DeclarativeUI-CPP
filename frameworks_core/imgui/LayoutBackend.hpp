#pragma once

#include "frameworks_core/ILayoutBackend.hpp"

#include <vector>

// ImGui adapter for the layout engine. Leaves draw immediate-mode at
// engine-computed rects inside the current window (one shared coordinate
// space — no BeginChild nesting). Group boxes are chrome drawn via the
// window draw list; tab panels map to BeginTabBar/BeginTabItem, and
// inactive pages report invisible so the engine skips their subtree.
class ImGuiLayoutBackend : public ILayoutBackend
{
public:
	Size measure(const LayoutNode& leaf, const Constraints& c) override;
	void place(const LayoutNode& leaf, const Rect& frame) override;
	EdgeInsets containerInsets(const LayoutNode& node) override;
	bool beginContainer(const LayoutNode& node, const Rect& frame) override;
	void endContainer(const LayoutNode& node) override;

private:
	// containers currently open, to recognize tab pages (children of an
	// open TabPanel) without parent pointers in LayoutNode
	std::vector<const LayoutNode*> m_containerStack;
};
