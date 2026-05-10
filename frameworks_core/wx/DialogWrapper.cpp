#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/GroupBoxWrapper.hpp"
#include "frameworks_core/LayoutWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <wx/wx.h>

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DialogWrapper::DialogWrapper()\t-> new wxDialog()\n");
#endif
	m_nativeWidget = new wxDialog(nullptr, wxID_ANY, title,
		wxDefaultPosition, wxSize(size.width, size.height));
}

void DialogWrapper::show()
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DialogWrapper::show()\t-> wxDialog->Show()\n");
#endif
	static_cast<wxDialog*>(m_nativeWidget)->Show();
}
