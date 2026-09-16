#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/imgui/FileBrowserPopup.hpp"
#include "frameworks_core/imgui/LayoutBackend.hpp"

#include <algorithm>

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include "imgui.h"

DialogWrapper::DialogWrapper(const std::string& title, const Size& size)
{
#ifdef USE_LOGGER
	Logger::instance().log("DialogWrapper::DialogWrapper()\t-> ImGui::Begin()\n");
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
	Logger::instance().log("DialogWrapper::show()\t-> ImGui::End()\n");
	Logger::instance().stopLogging();
#endif
	ImGui::End();
}

void DialogWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> rootPtr, bool resizable, const std::optional<Position>& position)
{
	// immediate mode: the tree lives for this frame only
	LayoutNode& root = *rootPtr;
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

	const ImVec2 winSize((float)(content.width + chromeW), (float)(content.height + chromeH));
	ImGuiWindowFlags winFlags = ImGuiWindowFlags_None;
	if (resizable)
	{
		// the measured content is the floor: the user can grow the window
		// but never shrink content into clipping
		const EdgeInsets margin = root.flags.border();
		const ImVec2 minWinSize(
			(float)(root.desired.width + margin.left + margin.right + chromeW),
			(float)(root.desired.height + margin.top + margin.bottom + chromeH));
		ImGui::SetNextWindowSizeConstraints(minWinSize, ImVec2(FLT_MAX, FLT_MAX));
		ImGui::SetNextWindowSize(winSize, ImGuiCond_FirstUseEver);
	}
	else
	{
		// the engine owns the window size; the user cannot resize it
		ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);
		winFlags |= ImGuiWindowFlags_NoResize;
	}

	// Once, not Always: this is where the window opens, so the user can still
	// drag it afterwards as they can on wx and Qt. Not FirstUseEver either --
	// that would defer to a position remembered in imgui.ini and silently
	// ignore the caller.
	if (position)
		ImGui::SetNextWindowPos(ImVec2((float)position->x, (float)position->y), ImGuiCond_Once);

	if (ImGui::Begin(title.c_str(), nullptr, winFlags))
	{
		Size renderContent = content;
		if (resizable)
		{
			// fill whatever size the user gave the window
			const ImVec2 actual = ImGui::GetWindowSize();
			renderContent.width = std::max(content.width, (int)actual.x - chromeW);
			renderContent.height = std::max(content.height, (int)actual.y - chromeH);
		}
		engine.render(root, renderContent);

		// The file browser is drawn HERE, not in the wrapper that asked for it:
		// ImGuiLayoutBackend::place() wraps every render() in BeginGroup() +
		// BeginDisabled(), and a modal begun there would inherit the disabled
		// item flags and sit inside a group it has nothing to do with. This is
		// the one point inside the window's ID scope but outside every group
		// and disabled scope, which is what lets OpenPopup and BeginPopupModal
		// meet in the same scope as ImGui requires.
		FileBrowser::drawPending();
	}
	ImGui::End();
}
