#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <string>
#include <vector>

#ifdef USE_WX
#include <wx/sizer.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

class LayoutFlags;
class ControlWrapper;

class LayoutWrapper
{
public:
	LayoutWrapper(Orientation orient);
	virtual ~LayoutWrapper();

	void add(LayoutFlags& flags);
	void add(LayoutWrapper* stack, LayoutFlags& flags);
	void add(ControlWrapper* widget, LayoutFlags& flags);

	virtual void finilizeLayout();

protected:
	Orientation m_orientation;

#ifdef USE_WX
public:
	explicit LayoutWrapper(wxSizer* sizer);
	wxSizer* nativeHandle() const;
protected:
	wxSizer* m_nativeSizer = nullptr;
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
protected:
	void saveCenterVCache();
private:
	bool m_firstChild = true;
	int m_lastBorderRight = 0;
	int m_centerVLayoutId = -1;
	int m_widgetIndex = 0;
	std::vector<float> m_currentHeights;
#endif

#ifdef USE_LOGGER
private:
	static inline int s_depth = 0;
public:
	static std::string indent();
#endif
};
