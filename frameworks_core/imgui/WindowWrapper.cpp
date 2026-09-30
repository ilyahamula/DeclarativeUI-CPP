#include "frameworks_core/WindowWrapper.hpp"

#include "frameworks_core/LayoutEngine.hpp"
#include "frameworks_core/LayoutNode.hpp"
#include "frameworks_core/imgui/AdoptedTopLevels.hpp"
#include "frameworks_core/imgui/DialogKeys.hpp"
#include "frameworks_core/imgui/FileBrowserPopup.hpp"
#include "frameworks_core/imgui/LayoutBackend.hpp"
#include "frameworks_core/imgui/ToastQueue.hpp"
#include "frameworks_core/UiThread.hpp"
#include "frameworks_core/imgui/MenuDraw.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include "imgui.h"

namespace
{

// A Window on ImGui is an ImGui window inside the host viewport, exactly as a
// Dialog is -- the mains own the GLFW window, and the framework draws into it.
// (Docking a Window to the whole viewport would be a Fullscreen() flag; it is
// not what a Window means here.)
//
// The tree is rebuilt every frame, so "has this window's close already been
// reported?" has no home in the wrapper. It lives here, keyed by title, which
// is the same thing ImGui keys the window's own state by -- so the two stay
// exactly as stable as each other.
struct WindowState
{
	bool open = true;      // used only when the caller bound no flag
	bool closeFired = false;
};

std::unordered_map<std::string, WindowState>& windowStates()
{
	static std::unordered_map<std::string, WindowState> states;
	return states;
}

} // unnamed namespace

void WindowWrapper::runLayoutEngine(const std::string& title, const Size& size,
	std::unique_ptr<LayoutNode> rootPtr, bool resizable, const MenuBarModel* menuBar,
	std::function<void()> onClose, bool* open)
{
	// Everything from here on -- the close callback below included -- is this
	// window's frame, so a show() issued in it is a handler's and is adopted
	// (TopLevelShow.hpp) rather than drawn as if it were the caller's frame.
	AdoptedTopLevels::FrameScope frameScope(title);
	// Default/cancel buttons offer their press into this while the tree draws.
	imgui_dialog_keys::WindowScope dialogKeys;

	// Toasts ride on whichever framework window the frame draws first, so an
	// app never has to call anything for them (ToastQueue.hpp). Here, before
	// anything can return early: a closed window's show() is still a frame.
	ToastQueue::draw();

	// Work a background thread posted (postToUi) runs here, inside this
	// window's frame, so a show() it issues is a handler's and is kept up.
	UiThreadQueue::drain();

	WindowState& state = windowStates()[title];

	// A Window has a close button on wx and Qt whether or not the caller bound
	// a flag to it, so it has one here too: with no caller flag the store owns
	// the bool. Either way ONE bool is the truth about whether the window is up.
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
		fireClose();
		return;
	}
	state.closeFired = false; // open (again): re-arm

	// immediate mode: the tree lives for this frame only
	LayoutNode& root = *rootPtr;
	ImGuiLayoutBackend backend;
	LayoutEngine engine(backend);

	// Window chrome around the engine's content space: padding, title bar, and
	// the menu row when there is one. ImGui has no chrome outside the window
	// the way wx and Qt do -- the bar is drawn INSIDE it -- so its height comes
	// straight off what the engine is given to lay out in.
	const bool hasMenuBar = menuBar != nullptr && !menuBar->empty();
	const ImGuiStyle& style = ImGui::GetStyle();
	const int chromeW = (int)(style.WindowPadding.x * 2.0f);
	int chromeH = (int)(style.WindowPadding.y * 2.0f + ImGui::GetFrameHeight());
	if (hasMenuBar)
		chromeH += (int)ImGui::GetFrameHeight();

	const ImVec2 display = ImGui::GetIO().DisplaySize;
	if (display.x > 0.0f)
		engine.setMaxAutoFitWidth((int)(display.x * 0.9f) - chromeW);

	// explicit Size means the total window size; auto-fit otherwise
	const bool fixedSize = size.width > 0 && size.height > 0;
	const Size contentRequest = fixedSize
		? Size { size.width - chromeW, size.height - chromeH }
		: Size { -1, -1 };
	const Size content = engine.resolve(root, contentRequest);

	const ImVec2 winSize((float)(content.width + chromeW), (float)(content.height + chromeH));
	ImGuiWindowFlags winFlags = hasMenuBar ? ImGuiWindowFlags_MenuBar : ImGuiWindowFlags_None;
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
		// Fixed(): the engine owns the window size
		ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);
		winFlags |= ImGuiWindowFlags_NoResize;
	}

	// `flag` goes straight to Begin, so the title bar's close button clears the
	// caller's own bool -- the same single truth the wx and Qt polls maintain.
	if (ImGui::Begin(title.c_str(), flag, winFlags))
	{
		// Drawn first, and inside Begin/End: the bar occupies the row the
		// chrome height above reserved for it, and routeMenuShortcuts() needs
		// this window to be the current one to route against its focus.
		if (hasMenuBar)
			drawMenuBar(*menuBar);

		Size renderContent = content;
		if (resizable)
		{
			// fill whatever size the user gave the window
			const ImVec2 actual = ImGui::GetWindowSize();
			renderContent.width = std::max(content.width, (int)actual.x - chromeW);
			renderContent.height = std::max(content.height, (int)actual.y - chromeH);
		}
		engine.render(root, renderContent);
		// Enter / Escape, now that every button has offered (DialogKeys.hpp).
		dialogKeys.dispatch();

		// The file browser is drawn HERE, not in the wrapper that asked for it:
		// ImGuiLayoutBackend::place() wraps every render() in BeginGroup() +
		// BeginDisabled(), and a modal begun there would inherit the disabled
		// item flags and sit inside a group it has nothing to do with. This is
		// the one point inside the window's ID scope but outside every group
		// and disabled scope, which is what lets OpenPopup and BeginPopupModal
		// meet in the same scope as ImGui requires.
		FileBrowser::drawPending();

		// Dialogs and Windows a handler in this frame (or an earlier one) showed,
		// drawn at the same point and for the same reason -- and, for a Modal()
		// child of a modal parent, nested inside the parent's popup as ImGui
		// requires. See AdoptedTopLevels.hpp.
		AdoptedTopLevels::drawAdopted();
	}
	ImGui::End();

	// the close button cleared it during this frame
	if (!*flag)
		fireClose();
}
