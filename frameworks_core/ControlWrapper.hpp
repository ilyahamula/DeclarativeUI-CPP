#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"
#include "LayoutFlags.hpp"
#include "LayoutWrapper.hpp"

class ControlWrapper
{
public:
	ControlWrapper();
	ControlWrapper(void* nativeWidget);
	ControlWrapper(const Position& pos, const Size& size, long style);
	virtual ~ControlWrapper();

	virtual void createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags);
	void setLayout(LayoutWrapper* layout);
	void* nativeHandle() const;

protected:
	void* m_nativeWidget = nullptr;
	Position m_pos { -1, -1 };
	Size m_size { -1, -1 };
	long m_style { 0 };
};
