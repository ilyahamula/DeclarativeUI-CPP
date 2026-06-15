#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/GroupBoxWrapper.hpp"
#include "frameworks_core/LayoutWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include "imgui.h"

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DialogWrapper::DialogWrapper()\t-> ImGui::Begin()\n");
#endif
	ImGuiWindowFlags flags = ImGuiWindowFlags_None;
	if (size.width > 0 && size.height > 0)
		ImGui::SetNextWindowSize(ImVec2((float)size.width, (float)size.height), ImGuiCond_FirstUseEver);
	else
		flags |= ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::Begin(title.c_str(), nullptr, flags);
}

void DialogWrapper::show()
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DialogWrapper::show()\t-> ImGui::End()\n");
	Logger::instance().stopLogging();
#endif
	ImGui::End();
}
