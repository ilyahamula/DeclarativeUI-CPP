#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <functional>
#include <optional>

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

	// Snapshot: the control is disabled iff `disabled` was true at build time.
	void setDisabled(const bool& disabled)
	{
		m_disabled = disabled;
		m_disabledRef.reset();
	}

	// Bind to a caller-owned flag instead. isDisabled() then reports the live
	// value, so an immediate backend picks the change up on its next frame for
	// free; retained backends poll the ref (see the wx LayoutBackend) because
	// they only apply the state once, when the native control is created.
	void setDisabled(bool& disabled)
	{
		m_disabled = disabled;
		m_disabledRef = disabled;
	}

	bool isDisabled() const
	{
		return m_disabledRef ? m_disabledRef->get() : m_disabled;
	}

	// Engaged when the disabled state is bound to a caller-owned flag.
	const std::optional<std::reference_wrapper<bool>>& disabledRef() const
	{
		return m_disabledRef;
	}

protected:
	void* m_nativeWidget = nullptr;
	Position m_pos { -1, -1 };
	Size m_size { -1, -1 };
	long m_style { 0 };
	bool m_disabled = false;
	std::optional<std::reference_wrapper<bool>> m_disabledRef;
};
