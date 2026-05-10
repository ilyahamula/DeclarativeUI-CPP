#include "frameworks_core/ControlWrapper.hpp"

ControlWrapper::ControlWrapper() = default;

ControlWrapper::~ControlWrapper() = default;

void ControlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	layout->add(this, flags);
}
