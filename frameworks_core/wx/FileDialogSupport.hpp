#pragma once

#include "frameworks_core/CoreTypes/FileFilter.hpp"
#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <string>
#include <vector>

#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/string.h>

// The wx half of T3.1's file support, shared by FileDialogWrapper (the one-shot
// FileDialog) and FilePickerWrapper (the Browse button). Both need the same two
// things -- the wildcard spelling and a modal run -- and neither should own them
// alone.

// Rebuild wx's wildcard string from the parsed fields.
//
// Note this NORMALISES rather than passes the caller's spec through: wx's
// grammar is what FileFilter::parse reads, so a round trip would be a no-op --
// except that a malformed spec has already been dropped by the parser, so what
// reaches wx here is always well-formed. That is the point. Re-parsing per
// backend is exactly what CoreTypes/FileFilter.hpp exists to avoid.
inline wxString wxWildcardFor(const std::vector<FileFilter>& filters)
{
	wxString wildcard;
	for (const FileFilter& filter : filters)
	{
		if (!wildcard.empty())
			wildcard += "|";
		wildcard += wxString::FromUTF8(filter.description) + "|";
		for (std::size_t i = 0; i < filter.patterns.size(); ++i)
		{
			if (i > 0)
				wildcard += ";";
			wildcard += wxString::FromUTF8(filter.patterns[i]);
		}
	}
	// wx shows every file when the wildcard is empty, which is what an empty
	// filter list means everywhere else too.
	return wildcard.empty() ? wxString("*.*") : wildcard;
}

// Run the modal dialog for `mode` and return the chosen path, or "" on cancel.
//
// `initialPath` seeds both halves: its directory is where the dialog opens and,
// for Save, its leaf name is the pre-filled file name. A path that is itself a
// directory seeds the directory only.
inline std::string wxRunFileDialog(wxWindow* parent, const std::string& title,
	FileMode mode, const std::vector<FileFilter>& filters, const std::string& initialPath)
{
	const wxString initial = wxString::FromUTF8(initialPath);

	if (mode == FileMode::Directory)
	{
		wxDirDialog dialog(parent,
			title.empty() ? wxString(wxDirSelectorPromptStr) : wxString::FromUTF8(title),
			initial);
		return dialog.ShowModal() == wxID_OK ? std::string(dialog.GetPath().ToUTF8()) : std::string();
	}

	wxString directory;
	wxString filename;
	if (!initial.empty())
	{
		if (wxDirExists(initial))
			directory = initial;
		else
		{
			const wxFileName parts(initial);
			directory = parts.GetPath();
			filename = parts.GetFullName();
		}
	}

	const long style = mode == FileMode::Save
		? (wxFD_SAVE | wxFD_OVERWRITE_PROMPT)
		: (wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	wxFileDialog dialog(parent,
		title.empty() ? wxString(wxFileSelectorPromptStr) : wxString::FromUTF8(title),
		directory, filename, wxWildcardFor(filters), style);
	return dialog.ShowModal() == wxID_OK ? std::string(dialog.GetPath().ToUTF8()) : std::string();
}
