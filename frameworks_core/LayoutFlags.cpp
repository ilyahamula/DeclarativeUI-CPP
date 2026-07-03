#include "frameworks_core/LayoutFlags.hpp"

LayoutFlags& LayoutFlags::Expand()
{
	m_expand = true;
	return *this;
}

LayoutFlags& LayoutFlags::Border(Side direction, int borderInPixels)
{
	if (direction & Side::Left)   m_border.left   = borderInPixels;
	if (direction & Side::Right)  m_border.right  = borderInPixels;
	if (direction & Side::Top)    m_border.top    = borderInPixels;
	if (direction & Side::Bottom) m_border.bottom = borderInPixels;
	return *this;
}

LayoutFlags& LayoutFlags::CenterVertical()
{
	m_centerVertical = true;
	return *this;
}

LayoutFlags& LayoutFlags::CenterHorizontal()
{
	m_centerHorizontal = true;
	return *this;
}

LayoutFlags& LayoutFlags::Center()
{
	m_centerVertical = true;
	m_centerHorizontal = true;
	return *this;
}

LayoutFlags& LayoutFlags::Proportion(int proportion)
{
	m_proportion = proportion;
	return *this;
}

LayoutFlags& LayoutFlags::MinSize(Size size)
{
	m_minSize = size;
	return *this;
}

LayoutFlags& LayoutFlags::MaxSize(Size size)
{
	m_maxSize = size;
	return *this;
}

LayoutFlags& LayoutFlags::SizeGroup(int id)
{
	m_sizeGroup = id;
	return *this;
}

LayoutFlags& LayoutFlags::AutoGrow()
{
	m_autoGrow = true;
	return *this;
}

int LayoutFlags::proportion() const
{
	return m_proportion;
}

Align LayoutFlags::crossAlign(NodeKind kind, Orientation parentOrientation) const
{
	// In a row the cross axis is vertical; in a column it is horizontal.
	const bool centered = parentOrientation == Orientation::Horizontal
		? m_centerVertical
		: m_centerHorizontal;
	if (centered)
		return Align::Center;
	if (m_expand)
		return Align::Stretch;
	return kind == NodeKind::Leaf ? Align::Start : Align::Stretch;
}

EdgeInsets LayoutFlags::border() const
{
	return m_border;
}

std::optional<Size> LayoutFlags::minSize() const
{
	return m_minSize;
}

std::optional<Size> LayoutFlags::maxSize() const
{
	return m_maxSize;
}

std::optional<int> LayoutFlags::sizeGroup() const
{
	return m_sizeGroup;
}

bool LayoutFlags::autoGrow() const
{
	return m_autoGrow;
}

bool LayoutFlags::expand() const { return m_expand; }
bool LayoutFlags::centerVertical() const { return m_centerVertical; }
bool LayoutFlags::centerHorizontal() const { return m_centerHorizontal; }
int LayoutFlags::borderLeft() const { return m_border.left; }
int LayoutFlags::borderRight() const { return m_border.right; }
int LayoutFlags::borderTop() const { return m_border.top; }
int LayoutFlags::borderBottom() const { return m_border.bottom; }
