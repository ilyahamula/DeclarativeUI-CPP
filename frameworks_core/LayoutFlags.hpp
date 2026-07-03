#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <optional>

// Pure-data description of how a node participates in its parent's box.
// No backend types or #ifdefs — exactly one engine interprets these fields
// identically on every backend (docs/specs/custom_layout_system/architecture.md).
class LayoutFlags
{
public:
	LayoutFlags() = default;

	LayoutFlags& Expand();                                // cross-axis Stretch
	LayoutFlags& Border(Side direction, int borderInPixels = 5);
	LayoutFlags& CenterVertical();                        // cross-axis Center in a row
	LayoutFlags& CenterHorizontal();                      // cross-axis Center in a column
	LayoutFlags& Center();                                // both of the above
	LayoutFlags& Proportion(int proportion);              // main-axis flex weight
	LayoutFlags& MinSize(Size size);
	LayoutFlags& MaxSize(Size size);
	LayoutFlags& SizeGroup(int id);                       // equalize desired size across nodes
	LayoutFlags& AutoGrow();                              // editable leaf re-measures live content

	// -- engine-facing accessors --------------------------------------------

	int proportion() const;

	// Resolves the cross-axis alignment of a node of `kind` inside a parent
	// with `parentOrientation`. An explicit Center* for the applicable axis
	// wins over Expand; Expand resolves to Stretch; otherwise the kind
	// default applies (Leaf -> Start, containers -> Stretch), so `Expand()`
	// on a container is a no-op.
	Align crossAlign(NodeKind kind, Orientation parentOrientation) const;

	EdgeInsets border() const;
	std::optional<Size> minSize() const;
	std::optional<Size> maxSize() const;
	std::optional<int> sizeGroup() const;
	bool autoGrow() const;

	// -- legacy accessors ----------------------------------------------------
	// Used by the pre-engine backends; deleted in Phase 4
	// (docs/specs/custom_layout_system/tasks.md, T4.1/T4.2).
	bool expand() const;
	bool centerVertical() const;
	bool centerHorizontal() const;
	int borderLeft() const;
	int borderRight() const;
	int borderTop() const;
	int borderBottom() const;

private:
	bool m_expand = false;
	bool m_centerVertical = false;
	bool m_centerHorizontal = false;
	int m_proportion = 0;
	EdgeInsets m_border;
	std::optional<Size> m_minSize;
	std::optional<Size> m_maxSize;
	std::optional<int> m_sizeGroup;
	bool m_autoGrow = false;
};
