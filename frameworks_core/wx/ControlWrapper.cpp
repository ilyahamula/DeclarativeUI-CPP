#include "frameworks_core/ControlWrapper.hpp"
#include <wx/wx.h>

ControlWrapper::ControlWrapper() = default;

ControlWrapper::~ControlWrapper() = default;

void ControlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	layout->add(this, flags);
}

ControlWrapper::ControlWrapper(wxWindow* window)
	: m_nativeWidget(window)
{
}

wxWindow* ControlWrapper::nativeHandle() const
{
	return m_nativeWidget;
}

void ControlWrapper::setLayout(LayoutWrapper* layout)
{
	if (m_nativeWidget)
		m_nativeWidget->SetSizerAndFit(layout->nativeHandle());
}
