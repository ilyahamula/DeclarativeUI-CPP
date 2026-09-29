#pragma once

#include <string>

#include <wx/string.h>

// Caller text shown as a wx control's LABEL. wx reads '&' in a label as the
// mnemonic marker -- on buttons, check boxes, radio buttons, static text,
// group boxes, notebook tabs and menu items alike -- so "Save & Exit" would
// lose its ampersand and underline the space after it. The framework offers
// no way to ask for a mnemonic, so every '&' the caller wrote is literal.
//
// The one spelling for every label on this backend, so no control can be the
// one that forgot.
inline wxString wxLabelText(const std::string& text)
{
	wxString out = wxString::FromUTF8(text);
	out.Replace(wxT("&"), wxT("&&"));
	return out;
}
