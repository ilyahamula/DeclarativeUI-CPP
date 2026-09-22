#pragma once

#include "frameworks_core/FileDialogWrapper.hpp"

#include <functional>
#include <string>

// A one-shot file dialog, the fluent twin of MessageBox: it is not a widget, it
// builds no node and it takes no part in a layout. Call it from a handler.
//
//     FileDialog{"Open project"}
//         .withMode(FileMode::Open)
//         .withFilter("Projects (*.dui)|*.dui|All files|*")
//         .onResult([&](const std::string& path) { if (!path.empty()) load(path); })
//         .show();
//
// onResult receives "" on cancel -- the framework's "no selection" spelling for
// a path -- so a result can be written straight through to a bound string.
struct FileDialog
{
	explicit FileDialog(const std::string& title)
		: m_title(title)
	{
	}

	// Open (default), Save or Directory.
	FileDialog& withMode(FileMode mode)
	{
		m_mode = mode;
		return *this;
	}

	// "Images (*.png;*.jpg)|*.png;*.jpg|All files|*" -- parsed once here and
	// mapped to each backend's own wildcard spelling (CoreTypes/FileFilter.hpp).
	// Ignored in Directory mode, where there are no files to filter.
	FileDialog& withFilter(const std::string& spec)
	{
		m_filters = FileFilter::parse(spec);
		return *this;
	}

	// Where the dialog opens: a directory, or a file whose directory and name
	// both seed it. Empty leaves the choice to the platform.
	FileDialog& withInitialPath(const std::string& path)
	{
		m_initialPath = path;
		return *this;
	}

	FileDialog& onResult(std::function<void(const std::string&)> callback)
	{
		m_onResult = std::move(callback);
		return *this;
	}

	// Blocking on wx and Qt. On ImGui this queues the framework-drawn browser
	// and returns at once -- nothing may block the frame loop -- so onResult
	// fires on a later frame. Call it from a click handler either way.
	void show()
	{
		FileDialogWrapper::show(m_title, m_mode, m_filters, m_initialPath, m_onResult);
	}

	// The per-frame spelling, as MessageBox has: `visible` says whether the
	// dialog should be up and is cleared once it has been opened.
	void show(bool& visible)
	{
		FileDialogWrapper::show(m_title, m_mode, m_filters, m_initialPath, m_onResult, visible);
	}

private:
	std::string m_title;
	FileMode m_mode = FileMode::Open;
	std::vector<FileFilter> m_filters;
	std::string m_initialPath;
	std::function<void(const std::string&)> m_onResult;
};
