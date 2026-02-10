#include "frameworks_core/LayoutFlags.hpp"

LayoutFlags::LayoutFlags()
	: m_flags()
{
}

LayoutFlags::LayoutFlags(int proportion)
	: m_flags(proportion)
{
}

LayoutFlags& LayoutFlags::Expand()
{
	m_flags.Expand();
	return *this;
}

LayoutFlags& LayoutFlags::Border(int direction, int borderInPixels)
{
	m_flags.Border(direction, borderInPixels);
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
