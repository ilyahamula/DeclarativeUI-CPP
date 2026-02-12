#pragma once

#ifdef USE_WX
#include <wx/sizer.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

class LayoutFlags
{
public:
	LayoutFlags();
	explicit LayoutFlags(int proportion);

	LayoutFlags& Expand();
	LayoutFlags& Border(int direction, int borderInPixels = 5);
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
#endif
};
