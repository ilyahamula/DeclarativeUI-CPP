#include "frameworks_core/DialogWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#ifdef USE_WX
#include <wx/wx.h>

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
#ifdef USE_LOGGER
	Logger::instance().log("DialogWrapper::DialogWrapper() -> new wxDialog()\n");
#endif
	m_nativeWidget = new wxDialog(nullptr, wxID_ANY, title,
		wxDefaultPosition, wxSize(size.width, size.height));
}

void DialogWrapper::show()
{
#ifdef USE_LOGGER
	Logger::instance().log("DialogWrapper::show() -> wxDialog->Show()\n");
#endif
	static_cast<wxDialog*>(m_nativeWidget)->Show();
}
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
#ifdef USE_LOGGER
	Logger::instance().log("DialogWrapper::DialogWrapper() -> ImGui::Begin()\n");
#endif
	ImGui::Begin(title.c_str());
}

void DialogWrapper::show()
{
#ifdef USE_LOGGER
	Logger::instance().log("DialogWrapper::show() -> ImGui::End()\n");
#endif
	ImGui::End();
}
#endif
