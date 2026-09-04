#pragma once

#include "frameworks_core/ILayoutBackend.hpp"

#include <map>
#include <memory>
#include <vector>

class wxWindow;

// wx adapter for the layout engine (retained widgets, absolute placement,
// no wxSizer). Leaves are realized (native control created) on first
// measure, measured with GetBestSize (plus content floors) and placed with
// SetSize. A leaf whose realize() creates no window is legal: it measures
// through the wrapper and is never placed, disabled or given a tooltip.
// GroupBox
// chrome is a wxStaticBox kept behind its sibling content; a TabPanel maps
// to a wxNotebook whose wxPanel pages become the parent and coordinate
// origin of their subtree — widgets are reparented into pages at place time.
class WxLayoutBackend : public ILayoutBackend
{
public:
	explicit WxLayoutBackend(wxWindow* host);

	Size measure(const LayoutNode& leaf, const Constraints& c) override;
	void place(const LayoutNode& leaf, const Rect& frame) override;
	EdgeInsets containerInsets(const LayoutNode& node) override;
	bool beginContainer(const LayoutNode& node, const Rect& frame) override;
	void endContainer(const LayoutNode& node) override;

private:
	struct Scope
	{
		const LayoutNode* node;
		wxWindow* parent;   // creation/placement parent for this subtree
		int originX;        // absolute origin of that parent's client space
		int originY;
	};

	wxWindow* currentParent() const { return m_stack.back().parent; }
	Rect toLocal(const Rect& frame) const;
	wxWindow* ensureContainer(const LayoutNode& node);

	wxWindow* m_host;
	std::map<const LayoutNode*, wxWindow*> m_containers;
	// Sampling policy: editable fields measure their INITIAL content only, so
	// the floor width is cached on first measure; AutoGrow leaves skip the
	// cache and re-measure live content on every pass.
	std::map<const LayoutNode*, int> m_textFloorWidths;
	std::vector<Scope> m_stack;
};
