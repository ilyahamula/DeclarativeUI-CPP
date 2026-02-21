#include "frameworks_core/ControlWrapper.hpp"

ControlWrapper::ControlWrapper() = default;

ControlWrapper::~ControlWrapper() = default;

void ControlWrapper::createAndAdd(ControlWrapper* parent, LayoutWrapper* layout, LayoutFlags flags)
{
	layout->add(this, flags);
}

#ifdef USE_WX
#include <wx/wx.h>

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
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"

void ControlWrapper::setLayout(LayoutWrapper* layout)
{
}
#endif
