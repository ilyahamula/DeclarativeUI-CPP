#pragma once

#include "frameworks_core/CoreTypes/FileFilter.hpp"
#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <functional>
#include <string>
#include <vector>

// A one-shot file dialog, on exactly the terms MessageBoxWrapper is one: a pair
// of static show()s, no wrapper instance, no node and no engine. A file dialog
// is not part of a layout -- it opens, the user answers, it closes -- so it
// never becomes a LayoutNode and never reaches ILayoutBackend.
//
// `onResult` receives the chosen path, or an EMPTY STRING on cancel. That is
// the same "no selection" spelling a path binding uses everywhere else in the
// framework (rules.md C5), so a caller can write the result straight through to
// a bound std::string without testing it first.
class FileDialogWrapper
{
public:
	// Blocking on wx and Qt -- the call returns once the user has answered.
	// On ImGui, where there is no OS dialog and nothing may block the frame
	// loop, this queues the framework-drawn browser and returns immediately;
	// `onResult` then fires on whichever later frame the user answers on.
	static void show(const std::string& title, FileMode mode,
		const std::vector<FileFilter>& filters, const std::string& initialPath,
		const std::function<void(const std::string&)>& onResult);

	// The per-frame spelling, matching MessageBoxWrapper's: `visible` is the
	// caller's "should this be up?" bool, cleared as soon as the dialog has
	// been opened so re-entering the frame loop does not open a second one.
	static void show(const std::string& title, FileMode mode,
		const std::vector<FileFilter>& filters, const std::string& initialPath,
		const std::function<void(const std::string&)>& onResult,
		bool& visible);
};
