#pragma once

#include "frameworks_core/CoreTypes/BoundValue.hpp"
#include "frameworks_core/CoreTypes/GeneralTypes.hpp"
#include "frameworks_core/CoreTypes/MenuModel.hpp"

#include <string>
#include <utility>

class ControlWrapper
{
public:
	ControlWrapper() = default;
	virtual ~ControlWrapper() = default;

	ControlWrapper(void* nativeWidget)
		: m_nativeWidget(nativeWidget)
	{
	}

	ControlWrapper(const Position& pos, const Size& size, long style)
		: m_pos(pos)
		, m_size(size)
		, m_style(style)
	{
	}

	// Create the native widget under `parentWindow` and bind its events
	// (retained backends: wxWindow* / QWidget*). Immediate backends create
	// nothing — they draw in render().
	virtual void realize(void* parentWindow)
	{
		(void)parentWindow;
	}

	// -- engine path (LayoutEngine / ILayoutBackend) --------------------------

	// Content size for the measure pass. An explicit withSize() dimension
	// overrides the intrinsic measurement per axis; the rest comes from
	// measureIntrinsic(). Non-const: measuring may cache (e.g. image dims).
	Size measureContent(const Constraints& c)
	{
		Size size = measureIntrinsic(c);
		if (m_size.width > 0)
			size.width = m_size.width;
		if (m_size.height > 0)
			size.height = m_size.height;
		return size;
	}

	// Intrinsic content size per the measurement contract table
	// (architecture.md). Backend-specific; the default is for widgets that
	// have no content-derived size.
	virtual Size measureIntrinsic(const Constraints&)
	{
		return Size { 0, 0 };
	}

	// Draw/realize the widget at the frame the engine assigned (the backend
	// adapter has already positioned the cursor/window). frame.width < 0
	// means "natural size" (legacy path).
	virtual void render(const Rect& frame)
	{
		(void)frame;
	}

	void* nativeHandle() const
	{
		return m_nativeWidget;
	}

	// The user-requested withSize() dimensions ({-1,-1} = none); retained
	// backends apply them as per-axis overrides of the native best size.
	const Size& explicitSize() const
	{
		return m_size;
	}

	// Snapshot or live binding, as the widget resolved it. Bound, isDisabled()
	// reports the live value, so an immediate backend picks a change up on its
	// next frame for free; retained backends poll the flag (see the wx
	// LayoutBackend) because they only apply the state once, at creation.
	void setDisabled(DisabledFlag disabled)
	{
		m_disabled = std::move(disabled);
	}

	bool isDisabled() const
	{
		return m_disabled.get();
	}

	// This widget's own flag. A disabled ancestor container also disables it,
	// which only the node tree knows -- see LayoutNode::isDisabledEffective().
	const DisabledFlag& disabledFlag() const
	{
		return m_disabled;
	}

	// Hover text, snapshot or live binding, exactly as setDisabled() takes the
	// disabled flag -- and for the same reason the copy is taken by value: the
	// widget that supplied it is a temporary that dies with the enclosing
	// declarative expression.
	void setTooltip(TooltipText tooltip)
	{
		m_tooltip = std::move(tooltip);
	}

	// Empty means "no tooltip". Bound, this reports the live text, so an
	// immediate backend picks an edit up on its next frame for free; retained
	// backends poll it (see the wx/Qt LayoutBackends) because they apply it
	// once, at creation.
	const std::string& tooltip() const
	{
		return m_tooltip.get();
	}

	// Non-null only while bound: the caller-owned string a retained backend has
	// to keep watching. A snapshot reports nullptr -- nothing can change it.
	const std::string* boundTooltip() const
	{
		return m_tooltip.boundValue();
	}

	// The leaf's right-click menu, on the same terms as the tooltip: copied by
	// value because the widget that supplied it is a temporary. Empty means
	// "no context menu", which is what every leaf that never asked for one has.
	void setContextMenu(ContextMenu menu)
	{
		m_contextMenu = std::move(menu);
	}

	// Read whenever a popup is about to open, never cached: a menu is rebuilt
	// from the model each time it is shown, so bound check and disabled flags
	// are picked up at that moment and nothing has to be polled -- the opposite
	// of the menu BAR, which is built once and outlives every read.
	const ContextMenu& contextMenu() const
	{
		return m_contextMenu;
	}

protected:
	void* m_nativeWidget = nullptr;
	Position m_pos { -1, -1 };
	Size m_size { -1, -1 };
	long m_style { 0 };
	DisabledFlag m_disabled;
	TooltipText m_tooltip;
	ContextMenu m_contextMenu;
};
