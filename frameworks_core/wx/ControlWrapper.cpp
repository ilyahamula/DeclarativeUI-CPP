#include "frameworks_core/ControlWrapper.hpp"
#include <wx/wx.h>

void ControlWrapper::setLayout(LayoutWrapper* layout)
{
	if (m_nativeWidget)
		reinterpret_cast<wxWindow*>(m_nativeWidget)->SetSizerAndFit(layout->nativeHandle());
}
