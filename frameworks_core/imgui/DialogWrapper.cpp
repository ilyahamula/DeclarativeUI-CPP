#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/GroupBoxWrapper.hpp"
#include "frameworks_core/LayoutWrapper.hpp"

#ifdef USE_LAYOUT_ENGINE
#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/imgui/LayoutBackend.hpp"
#endif

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

#ifdef USE_LAYOUT_ENGINE
void DialogWrapper::runLayoutEngine(const std::string& title, const Size& size, LayoutNode& root)
{
	ImGuiLayoutBackend backend;
	LayoutEngine engine(backend);

	// window chrome around the engine's content space: padding + title bar
	const ImGuiStyle& style = ImGui::GetStyle();
	const int chromeW = (int)(style.WindowPadding.x * 2.0f);
	const int chromeH = (int)(style.WindowPadding.y * 2.0f + ImGui::GetFrameHeight());

	const ImVec2 display = ImGui::GetIO().DisplaySize;
	if (display.x > 0.0f)
		engine.setMaxAutoFitWidth((int)(display.x * 0.9f) - chromeW);

	// explicit Size means the total window size; auto-fit otherwise
	const bool fixed = size.width > 0 && size.height > 0;
	const Size contentRequest = fixed
		? Size { size.width - chromeW, size.height - chromeH }
		: Size { -1, -1 };
	const Size content = engine.resolve(root, contentRequest);

	// the engine owns the window size; the user cannot resize it
	ImGui::SetNextWindowSize(
		ImVec2((float)(content.width + chromeW), (float)(content.height + chromeH)),
		ImGuiCond_Always);
	if (ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoResize))
		engine.render(root, content);
	ImGui::End();
}
#endif
