#include "frameworks_core/DialogWrapper.hpp"
#include "frameworks_core/ControlWrappers.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/imgui/FileBrowserPopup.hpp"
#include "frameworks_core/imgui/LayoutBackend.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>

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

namespace
{

// The tree is rebuilt every frame, so "is this dialog still up?" and "has its
// close already been reported?" have no home in the wrapper. They live here,
// keyed by title, which is the same thing ImGui keys the window's own state by
// -- so the two stay exactly as stable as each other. The identical move
// imgui/WindowWrapper.cpp makes, for the identical reason.
struct DialogState
{
	bool open = true;      // used only when the caller bound no flag
	bool closeFired = false;
};

std::unordered_map<std::string, DialogState>& dialogStates()
{
	static std::unordered_map<std::string, DialogState> states;
	return states;
}

} // unnamed namespace

void DialogWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> rootPtr, bool resizable, const std::optional<Position>& position,
	bool modal, std::function<void()> onClose, bool* open)
{
	DialogState& state = dialogStates()[title];

	// A dialog has a close button on wx and Qt whether or not the caller bound
	// a flag to it, so it has one here too: with no caller flag the store owns
	// the bool. Either way ONE bool is the truth about whether the dialog is up,
	// which is also what gives onClose() something to fire on.
	bool* flag = open != nullptr ? open : &state.open;

	auto fireClose = [&state, &onClose] {
		if (state.closeFired)
			return;
		state.closeFired = true;
		if (onClose)
			onClose();
	};

	if (!*flag)
	{
		// Closed, but the caller still calls show() every frame: draw nothing
		// and report the close exactly once.
		//
		// A modal needs one last BeginPopupModal to actually GO, though, and
		// this is that call. ImGui never closes a popup merely because nobody
		// submitted it -- GetTopMostPopupModal() reads the entry, not the
		// window's activity -- so a popup left in the stack would keep blocking
		// input to everything behind a dialog that has already reported itself
		// closed. Handed the cleared flag, BeginPopupModal pops the entry and
		// returns false, which is why nothing pairs an EndPopup with it.
		if (modal && ImGui::IsPopupOpen(title.c_str()))
		{
			if (ImGui::BeginPopupModal(title.c_str(), flag))
				ImGui::EndPopup();
		}
		fireClose();
		return;
	}
	state.closeFired = false; // open (again): re-arm

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

	// Modal() is a different CALL, not another window flag: only a popup dims
	// the windows behind it and refuses them input. OpenPopup and
	// BeginPopupModal have to meet in one ID scope, and the caller's show() --
	// outside every window, group and disabled scope -- is that scope.
	//
	// Asked to open only while it is not already open. ImGui treats an
	// OpenPopup() every frame as a programming mistake and suppresses the
	// reopen to stay usable; leaning on that would mean leaning on a fallback.
	//
	// `flag` goes to both spellings, so the title bar's close button clears the
	// caller's own bool -- the same single truth the wx and Qt polls maintain.
	bool visible = false;
	if (modal)
	{
		if (!ImGui::IsPopupOpen(title.c_str()))
			ImGui::OpenPopup(title.c_str());
		visible = ImGui::BeginPopupModal(title.c_str(), flag, winFlags);
	}
	else
	{
		visible = ImGui::Begin(title.c_str(), flag, winFlags);
	}
	if (visible)
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
	// EndPopup pairs with a BeginPopupModal that RETURNED TRUE -- it closes
	// itself on the way out otherwise -- while End pairs with Begin either way.
	if (modal)
	{
		if (visible)
			ImGui::EndPopup();
	}
	else
	{
		ImGui::End();
	}

	// the close button cleared it during this frame
	if (!*flag)
		fireClose();
}
