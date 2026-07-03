#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

struct LayoutNode;

// The only backend-specific layout surface: intrinsic measurement, absolute
// placement, and container chrome. The engine owns all geometry math; an
// implementation of this interface must not reinterpret LayoutFlags
// (docs/specs/custom_layout_system/architecture.md).
class ILayoutBackend
{
public:
	virtual ~ILayoutBackend() = default;

	// Intrinsic content size of a leaf widget under the given constraints.
	// Implements the per-widget measurement contract (content extent with
	// floors; wrapping honors c.maxWidth). Most widgets ignore `c`.
	virtual Size measure(const LayoutNode& leaf, const Constraints& c) = 0;

	// Position + size a leaf at an absolute rect, and draw/realize it.
	virtual void place(const LayoutNode& leaf, const Rect& frame) = 0;

	// Chrome thickness a container adds around its content (group-box
	// border + title, tab-bar height). The engine adds it to the
	// container's desired size and insets the children's area by it.
	virtual EdgeInsets containerInsets(const LayoutNode& node) = 0;

	// Container chrome: group-box border + title, tab bar; receives the
	// container's final frame. Called around the container's children.
	// Returns whether the children are visible — a tab panel's inactive
	// pages return false and the engine skips that subtree. endContainer
	// is called only when beginContainer returned true.
	virtual bool beginContainer(const LayoutNode& node, const Rect& frame) = 0;
	virtual void endContainer(const LayoutNode& node) = 0;
};
