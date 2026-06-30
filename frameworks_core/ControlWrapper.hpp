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
