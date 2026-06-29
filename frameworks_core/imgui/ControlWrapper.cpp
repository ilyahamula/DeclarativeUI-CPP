#include "frameworks_core/ControlWrapper.hpp"
#include "imgui.h"

ControlWrapper::ControlWrapper() = default;

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
