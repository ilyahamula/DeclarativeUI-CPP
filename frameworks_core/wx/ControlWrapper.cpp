#include "frameworks_core/ControlWrapper.hpp"
#include <wx/wx.h>

ControlWrapper::ControlWrapper() = default;

ControlWrapper::~ControlWrapper() = default;

ControlWrapper::ControlWrapper(void* widget)
	: m_nativeWidget(widget)
{
}

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
	if (m_nativeWidget)
		reinterpret_cast<wxWindow*>(m_nativeWidget)->SetSizerAndFit(layout->nativeHandle());
}
