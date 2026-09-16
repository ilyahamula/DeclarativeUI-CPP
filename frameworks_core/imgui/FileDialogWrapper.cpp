#include "frameworks_core/FileDialogWrapper.hpp"
#include "frameworks_core/imgui/FileBrowserPopup.hpp"

// ImGui has no OS dialog and nothing here may block the frame loop, so both
// show()s QUEUE the framework-drawn browser and return at once; onResult fires
// on whichever later frame the user answers on. wx and Qt block instead, which
// is the one behavioural difference between the backends here -- a caller that
// does work after show() sees it run before the answer on ImGui and after it on
// the other two, so the answer belongs in onResult on all three.

void FileDialogWrapper::show(const std::string& title, FileMode mode,
	const std::vector<FileFilter>& filters, const std::string& initialPath,
	const std::function<void(const std::string&)>& onResult)
{
	FileBrowser::request(title, mode, filters, initialPath, onResult);
}

void FileDialogWrapper::show(const std::string& title, FileMode mode,
	const std::vector<FileFilter>& filters, const std::string& initialPath,
	const std::function<void(const std::string&)>& onResult,
	bool& visible)
{
	if (!visible)
		return;
	// Cleared as soon as the request is queued: the caller rebuilds this tree
	// every frame, so leaving it set would re-request for as long as the
	// browser was up.
	visible = false;
	FileBrowser::request(title, mode, filters, initialPath, onResult);
}
