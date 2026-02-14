#include "frameworks_core/DialogWrapper.hpp"

#ifdef USE_WX
#include <wx/wx.h>
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
#ifdef USE_WX
	m_nativeWidget = new wxDialog(nullptr, wxID_ANY, title,
		wxDefaultPosition, wxSize(size.width, size.height));
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
}

void DialogWrapper::show()
{
#ifdef USE_WX
	static_cast<wxDialog*>(m_nativeWidget)->Show();
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#endif
}
