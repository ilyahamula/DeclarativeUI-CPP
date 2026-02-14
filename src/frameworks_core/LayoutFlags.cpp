#include "frameworks_core/LayoutFlags.hpp"

#ifdef USE_WX
static int toWxDirection(Side side)
{
	int dir = 0;
	if (side & Side::Left)   dir |= wxLEFT;
	if (side & Side::Right)  dir |= wxRIGHT;
	if (side & Side::Top)    dir |= wxTOP;
	if (side & Side::Bottom) dir |= wxBOTTOM;
	return dir;
}
#endif

LayoutFlags::LayoutFlags()
#ifdef USE_WX
	: m_flags()
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
{
}

LayoutFlags::LayoutFlags(int proportion)
#ifdef USE_WX
	: m_flags(proportion)
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
{
}

LayoutFlags& LayoutFlags::Expand()
{
#ifdef USE_WX
	m_flags.Expand();
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
	return *this;
}

LayoutFlags& LayoutFlags::Border(Side direction, int borderInPixels)
{
#ifdef USE_WX
	m_flags.Border(toWxDirection(direction), borderInPixels);
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
	return *this;
}

LayoutFlags& LayoutFlags::CenterVertical()
{
#ifdef USE_WX
	m_flags.CenterVertical();
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
	return *this;
}

LayoutFlags& LayoutFlags::CenterHorizontal()
{
#ifdef USE_WX
	m_flags.CenterHorizontal();
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
	return *this;
}

LayoutFlags& LayoutFlags::Center()
{
#ifdef USE_WX
	m_flags.Center();
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
	return *this;
}

LayoutFlags& LayoutFlags::Proportion(int proportion)
{
#ifdef USE_WX
	m_flags.Proportion(proportion);
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
	return *this;
}

#ifdef USE_WX
const wxSizerFlags& LayoutFlags::wx() const
{
	return m_flags;
}
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
