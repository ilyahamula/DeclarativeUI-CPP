#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"
#include "LayoutFlags.hpp"
#include "LayoutWrapper.hpp"

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

	virtual void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
	{
		layout->add(this, flags);
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

	void setLayout(LayoutWrapper* layout);

protected:
	void* m_nativeWidget = nullptr;
	Position m_pos { -1, -1 };
	Size m_size { -1, -1 };
	long m_style { 0 };
};
