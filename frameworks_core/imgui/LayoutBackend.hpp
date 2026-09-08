#pragma once

#include "frameworks_core/ILayoutBackend.hpp"

#include <vector>

// ImGui adapter for the layout engine. Leaves draw immediate-mode at
// engine-computed rects. Group boxes are chrome drawn via the window draw
// list; tab panels map to BeginTabBar/BeginTabItem, and inactive pages report
// invisible so the engine skips their subtree.
//
// Coordinates are one shared space per ImGui window, and a ScrollPanel is the
// one thing that opens another: BeginChild gives the panel its own cursor
// origin, so the backend keeps an origin stack (mirroring the parent+origin
// scopes wx and Qt use for tab pages) and toWindowPos subtracts the innermost
// origin from the engine's absolute frames.
class ImGuiLayoutBackend : public ILayoutBackend
{
public:
	Size measure(const LayoutNode& leaf, const Constraints& c) override;
	void place(const LayoutNode& leaf, const Rect& frame) override;
	EdgeInsets containerInsets(const LayoutNode& node) override;
	bool beginContainer(const LayoutNode& node, const Rect& frame) override;
	void endContainer(const LayoutNode& node) override;

private:
	// The coordinate space frames are expressed in. Only a ScrollPanel pushes
	// one; everything else shares its parent's, which is why the origin is the
	// panel's own absolute origin rather than a running sum.
	struct Scope
	{
		const LayoutNode* node;
		int originX;
		int originY;
	};

	const Scope& currentScope() const { return m_originStack.back(); }

	std::vector<Scope> m_originStack { Scope { nullptr, 0, 0 } };

	// containers currently open, to recognize tab pages (children of an
	// open TabPanel)
	std::vector<const LayoutNode*> m_containerStack;
	// whether each open container pushed a BeginDisabled scope, so
	// endContainer pops exactly the ones beginContainer pushed
	std::vector<bool> m_disabledStack;
};
