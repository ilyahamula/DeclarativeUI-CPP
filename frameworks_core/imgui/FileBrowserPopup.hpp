#pragma once

#include "frameworks_core/CoreTypes/FileFilter.hpp"
#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <functional>
#include <string>
#include <vector>

// The framework-drawn file browser (USE_IMGUI only).
//
// ImGui has no OS file dialog and this project takes no new dependency for one
// (tinyfiledialogs was considered and rejected), so Browse draws its own: a
// modal over std::filesystem with a breadcrumb, a directory listing, a filter
// combo, a file-name field for Save and Open/Cancel. It is deliberately simpler
// than the native dialogs wx and Qt open -- no favourites, no previews, no
// network places. That divergence is confirmed in design.md §12.
//
// WHERE IT IS DRAWN is the load-bearing detail. Not in the requesting wrapper's
// render(): ImGuiLayoutBackend::place() wraps every render() in BeginGroup() +
// BeginDisabled(), and a modal begun there would inherit the disabled item
// flags and sit inside a group it has nothing to do with. It is drawn instead
// by the top-level window, right after engine.render() and before ImGui::End()
// (imgui/DialogWrapper.cpp, imgui/WindowWrapper.cpp) -- the one point inside
// the window's ID scope but outside every group and disabled scope, so
// OpenPopup and BeginPopupModal meet in the same scope as ImGui requires.
//
// The request owns its completion callback rather than the wrapper doing so.
// It has to: the tree is rebuilt every frame, so the wrapper that opened the
// browser is long gone by the time the user picks a file.
namespace FileBrowser
{

// Queue the browser. At most one request is live at a time, because at most one
// modal can be -- a second request while one is up is dropped rather than
// queued, since by the time it could run, the frame that asked for it is gone.
void request(const std::string& title, FileMode mode,
	std::vector<FileFilter> filters, std::string initialPath,
	std::function<void(const std::string&)> onResult);

// Is a browser open or waiting to open?
bool pending();

// Draw the pending browser, if any. Called once per top-level window per frame
// from outside every group and disabled scope; a no-op when nothing is pending.
void drawPending();

} // namespace FileBrowser
