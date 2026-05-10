#include "frameworks_core/LayoutFlags.hpp"

static int toWxDirection(Side side)
{
	int dir = 0;
	if (side & Side::Left)   dir |= wxLEFT;
	if (side & Side::Right)  dir |= wxRIGHT;
	if (side & Side::Top)    dir |= wxTOP;
	if (side & Side::Bottom) dir |= wxBOTTOM;
	return dir;
}

LayoutFlags::LayoutFlags()
	: m_flags()
{
}

LayoutFlags& LayoutFlags::Expand()
{
	m_flags.Expand();
	return *this;
}

LayoutFlags& LayoutFlags::Border(Side direction, int borderInPixels)
{
	m_flags.Border(toWxDirection(direction), borderInPixels);
	return *this;
}

LayoutFlags& LayoutFlags::CenterVertical()
{
	m_flags.CenterVertical();
	return *this;
}

LayoutFlags& LayoutFlags::CenterHorizontal()
{
	m_flags.CenterHorizontal();
	return *this;
}

LayoutFlags& LayoutFlags::Center()
{
	m_flags.Center();
	return *this;
}

LayoutFlags& LayoutFlags::Proportion(int proportion)
{
	m_flags.Proportion(proportion);
	return *this;
}

const wxSizerFlags& LayoutFlags::wx() const
{
	return m_flags;
}
