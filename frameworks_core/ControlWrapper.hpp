#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"
#include "LayoutFlags.hpp"
#include "LayoutWrapper.hpp"

#ifdef USE_WX
#include <wx/window.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

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
