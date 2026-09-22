#include "frameworks_core/FileDialogWrapper.hpp"
#include "frameworks_core/wx/FileDialogSupport.hpp"

#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>

namespace
{

void showImpl(const std::string& title, FileMode mode,
	const std::vector<FileFilter>& filters, const std::string& initialPath,
	const std::function<void(const std::string&)>& onResult)
{
	const std::string chosen = wxRunFileDialog(nullptr, title, mode, filters, initialPath);
	// "" on cancel, which is the framework's "no selection" spelling for a
	// path -- so the callback fires either way and the caller tests the string
	// rather than needing a separate "cancelled" signal.
	if (onResult)
		onResult(chosen);
}

} // unnamed namespace

void FileDialogWrapper::show(const std::string& title, FileMode mode,
	const std::vector<FileFilter>& filters, const std::string& initialPath,
	const std::function<void(const std::string&)>& onResult)
{
	showImpl(title, mode, filters, initialPath, onResult);
}

void FileDialogWrapper::show(const std::string& title, FileMode mode,
	const std::vector<FileFilter>& filters, const std::string& initialPath,
	const std::function<void(const std::string&)>& onResult,
	bool& visible)
{
	if (!visible)
		return;
	// Cleared BEFORE the dialog runs, not after: ShowModal spins its own event
	// loop, so the caller's frame code can re-enter this while the dialog is
	// still up and would open a second one.
	visible = false;
	showImpl(title, mode, filters, initialPath, onResult);
}
