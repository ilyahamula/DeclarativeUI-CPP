#include "frameworks_core/LayoutFlags.hpp"

LayoutFlags::LayoutFlags()
{
}

LayoutFlags& LayoutFlags::Expand()
{
	m_expand = true;
	return *this;
}

LayoutFlags& LayoutFlags::Border(Side direction, int borderInPixels)
{
	if (direction & Side::Left)   m_borderLeft   = borderInPixels;
	if (direction & Side::Right)  m_borderRight  = borderInPixels;
	if (direction & Side::Top)    m_borderTop    = borderInPixels;
	if (direction & Side::Bottom) m_borderBottom = borderInPixels;
	return *this;
}

LayoutFlags& LayoutFlags::CenterVertical()
{
	m_centerVertical = true;
	return *this;
}

LayoutFlags& LayoutFlags::CenterHorizontal()
{
	return *this;
}

LayoutFlags& LayoutFlags::Center()
{
	return *this;
}

LayoutFlags& LayoutFlags::Proportion(int proportion)
{
	m_proportion = proportion;
	return *this;
}

bool LayoutFlags::expand() const { return m_expand; }
bool LayoutFlags::centerVertical() const { return m_centerVertical; }
int LayoutFlags::proportion() const { return m_proportion; }
int LayoutFlags::borderLeft() const { return m_borderLeft; }
int LayoutFlags::borderRight() const { return m_borderRight; }
int LayoutFlags::borderTop() const { return m_borderTop; }
int LayoutFlags::borderBottom() const { return m_borderBottom; }
