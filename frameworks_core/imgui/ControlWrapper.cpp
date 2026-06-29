#include "frameworks_core/ControlWrapper.hpp"
#include "imgui.h"

ControlWrapper::ControlWrapper() = default;

ControlWrapper::ControlWrapper(const Position& pos, const Size& size, long style)
	: m_pos(pos)
	, m_size(size)
	, m_style(style)
{
}

ControlWrapper::~ControlWrapper() = default;

void ControlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	layout->add(this, flags);
}

void* ControlWrapper::nativeHandle() const
{
	return m_nativeWidget;
}

void ControlWrapper::setLayout(LayoutWrapper* layout)
{
}
