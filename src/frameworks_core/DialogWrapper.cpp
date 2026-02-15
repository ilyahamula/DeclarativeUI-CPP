#include "frameworks_core/DialogWrapper.hpp"

#ifdef USE_WX
#include <wx/wx.h>

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
	m_nativeWidget = new wxDialog(nullptr, wxID_ANY, title,
		wxDefaultPosition, wxSize(size.width, size.height));
}

void DialogWrapper::show()
{
	static_cast<wxDialog*>(m_nativeWidget)->Show();
}
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
	ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
}

void DialogWrapper::show()
{
	ImGui::End();
}
#endif
