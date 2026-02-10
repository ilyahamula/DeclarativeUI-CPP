#pragma once

#include <wx/sizer.h>

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

	const wxSizerFlags& wx() const;

private:
	wxSizerFlags m_flags;
};
