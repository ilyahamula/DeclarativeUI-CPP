#pragma once

#include "frameworks_core/ILayoutBackend.hpp"

#include <map>
#include <memory>
#include <vector>

class QWidget;

// Qt adapter for the layout engine (retained widgets, absolute placement,
// no QLayout). Leaves are realized (native widget created) on first measure,
// measured with sizeHint() (plus content floors from QFontMetrics) and
// placed with setGeometry(). GroupBox
// chrome is a QGroupBox kept behind its sibling content; a TabPanel maps to
// a QTabWidget whose pages become the parent and coordinate origin of their
// subtree — widgets are reparented into pages at place time.
class QtLayoutBackend : public ILayoutBackend
{
public:
	explicit QtLayoutBackend(QWidget* host);

	Size measure(const LayoutNode& leaf, const Constraints& c) override;
	void place(const LayoutNode& leaf, const Rect& frame) override;
	EdgeInsets containerInsets(const LayoutNode& node) override;
	bool beginContainer(const LayoutNode& node, const Rect& frame) override;
	void endContainer(const LayoutNode& node) override;

private:
	struct Scope
	{
		const LayoutNode* node;
		QWidget* parent;   // creation/placement parent for this subtree
		int originX;       // absolute origin of that parent's client space
		int originY;
	};

	QWidget* currentParent() const { return m_stack.back().parent; }
	Rect toLocal(const Rect& frame) const;
	QWidget* ensureContainer(const LayoutNode& node);

	QWidget* m_host;
	std::map<const LayoutNode*, QWidget*> m_containers;
	// Sampling policy: editable fields measure their INITIAL content only
	// (cached); AutoGrow leaves re-measure live content on every pass.
	std::map<const LayoutNode*, int> m_textFloorWidths;
	std::vector<Scope> m_stack;
};
