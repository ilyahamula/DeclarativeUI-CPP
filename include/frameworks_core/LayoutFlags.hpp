#pragma once

#include "GeneralTypes.hpp"

#ifdef USE_WX
#include <wx/sizer.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

class LayoutFlags
{
public:
	LayoutFlags();

	LayoutFlags& Expand();
	LayoutFlags& Border(Side direction, int borderInPixels = 5);
	LayoutFlags& CenterVertical();
	LayoutFlags& CenterHorizontal();
	LayoutFlags& Center();
	LayoutFlags& Proportion(int proportion);

#ifdef USE_WX
	const wxSizerFlags& wx() const;
private:
	wxSizerFlags m_flags;
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
	bool expand() const;
	int proportion() const;
	int borderLeft() const;
	int borderRight() const;
	int borderTop() const;
	int borderBottom() const;
private:
	bool m_expand = false;
	int m_proportion = 0;
	int m_borderLeft = 0;
	int m_borderRight = 0;
	int m_borderTop = 0;
	int m_borderBottom = 0;
#endif
};
