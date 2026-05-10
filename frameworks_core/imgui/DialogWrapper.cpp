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
	ImGui::Begin(title.c_str());
}

void DialogWrapper::show()
{
#ifdef USE_LOGGER
	Logger::instance().log(LayoutWrapper::indent() + "DialogWrapper::show()\t-> ImGui::End()\n");
	Logger::instance().stopLogging();
#endif
	ImGui::End();
}
