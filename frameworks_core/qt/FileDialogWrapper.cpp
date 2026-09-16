#include "frameworks_core/FileDialogWrapper.hpp"
#include "frameworks_core/qt/FileDialogSupport.hpp"

void FileDialogWrapper::show(const std::string& title, FileMode mode,
	const std::vector<FileFilter>& filters, const std::string& initialPath,
	const std::function<void(const std::string&)>& onResult)
{
	const std::string chosen = qtRunFileDialog(nullptr, title, mode, filters, initialPath);
	// "" on cancel, the framework's "no selection" spelling for a path -- so
	// the callback fires either way and the caller tests the string rather than
	// needing a separate "cancelled" signal.
	if (onResult)
		onResult(chosen);
}

void FileDialogWrapper::show(const std::string& title, FileMode mode,
	const std::vector<FileFilter>& filters, const std::string& initialPath,
	const std::function<void(const std::string&)>& onResult,
	bool& visible)
{
	if (!visible)
		return;
	// Cleared BEFORE the dialog runs, not after: the static QFileDialog calls
	// spin a nested event loop, so the caller's code can re-enter this while
	// the dialog is still up and would open a second one.
	visible = false;
	show(title, mode, filters, initialPath, onResult);
}
