#include "frameworks_core/wx/MenuBuilder.hpp"

#include "frameworks_core/wx/RefSync.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/string.h>

#include <string>
#include <utility>

namespace
{

// The copied model plus the frame's menu bar. Lives on the heap until the frame
// is destroyed: every event handler and RefSync poll below holds a MenuItem*
// into `model`, and those stay valid because nothing mutates the vectors after
// the bar is built.
struct MenuHost
{
	MenuBarModel model;
	wxMenuBar* bar = nullptr;
};

// wx treats '&' as the mnemonic marker in menu labels exactly as it does in
// container labels, so caller text has to escape it -- the same rule, and the
// same two characters, as wx/LayoutBackend.cpp applies.
wxString escaped(const std::string& text)
{
	wxString out = wxString::FromUTF8(text.c_str());
	out.Replace(wxT("&"), wxT("&&"));
	return out;
}

// wx builds the frame's accelerator table by re-parsing the text after '\t', so
// the key has to be spelled the way wxAcceleratorEntry::ParseAccel spells it --
// which is not how Qt or ImGui spell it. Each backend owns its own table for
// exactly this reason.
//
// `super` is deliberately absent: wx has no portable spelling for it, and a
// modifier that registered on one backend only would break the promise that one
// shortcut string behaves the same everywhere (CoreTypes/Shortcut.hpp).
wxString acceleratorText(const Shortcut& shortcut)
{
	wxString out;
	if (shortcut.ctrl)
		out += wxT("Ctrl+");
	if (shortcut.alt)
		out += wxT("Alt+");
	if (shortcut.shift)
		out += wxT("Shift+");

	const std::string& key = shortcut.key;
	if (key == "Escape")        out += wxT("Esc");
	else if (key == "Delete")   out += wxT("Del");
	else if (key == "Insert")   out += wxT("Ins");
	else if (key == "PageUp")   out += wxT("PgUp");
	else if (key == "PageDown") out += wxT("PgDn");
	else if (key == "Backspace")out += wxT("Back");
	else if (key == "Plus")     out += wxT("+");
	else if (key == "Minus")    out += wxT("-");
	else if (key == "Comma")    out += wxT(",");
	else if (key == "Period")   out += wxT(".");
	else if (key == "Slash")    out += wxT("/");
	else                        out += wxString::FromUTF8(key.c_str());
	return out;
}

void buildMenu(wxMenu* menu, std::vector<MenuItem>& items, MenuHost* host, wxFrame* frame);

void appendItem(wxMenu* menu, MenuItem& item, MenuHost* host, wxFrame* frame)
{
	if (item.isSeparator)
	{
		menu->AppendSeparator();
		return;
	}

	if (!item.submenu.empty())
	{
		// A submenu's own item is a container: it opens, it never selects.
		auto* sub = new wxMenu;
		buildMenu(sub, item.submenu, host, frame);
		menu->AppendSubMenu(sub, escaped(item.label));
		return;
	}

	const int id = wxWindow::NewControlId();
	wxString text = escaped(item.label);
	if (item.shortcut)
		text += wxT("\t") + acceleratorText(*item.shortcut);
#ifdef USE_LOGGER
	else if (!item.shortcutText.empty())
		Logger::instance().log("attachMenuBar()\t-> unparseable shortcut \""
			+ item.shortcutText + "\" on \"" + item.label + "\": no accelerator registered\n");
#endif

	const bool checkable = item.checkedFlag.has_value();
	if (checkable)
		menu->AppendCheckItem(id, text);
	else
		menu->Append(id, text);

	MenuItem* model = &item;
	if (checkable)
		menu->Check(id, model->checkedFlag->get());
	menu->Enable(id, !model->disabledFlag.get());

	frame->Bind(wxEVT_MENU, [model, host, id](wxCommandEvent& event) {
		// Value first, then the callback, so a handler reading the bound bool
		// sees the state the user just selected (the rule every control follows).
		if (model->checkedFlag)
			model->checkedFlag->set(host->bar->IsChecked(id));
		if (model->selectHandler)
			model->selectHandler();
		event.Skip(false);
	}, id);

	// Bound flags are polled, never pushed on disagreement: wx has no
	// notification for a bool written somewhere else, and a check mark the user
	// just clicked must not be fought by the poll that notices it.
	if (checkable && model->checkedFlag->isBound())
	{
		const bool* flag = model->checkedFlag->boundValue();
		bindExternalRefSync(frame,
			[host, id] { return host->bar->IsChecked(id); },
			[flag] { return *flag; },
			[host, id](bool value) { host->bar->Check(id, value); });
	}
	if (model->disabledFlag.isBound())
	{
		const bool* flag = model->disabledFlag.boundValue();
		bindExternalRefSync(frame,
			[host, id] { return host->bar->IsEnabled(id); },
			[flag] { return !*flag; },
			[host, id](bool enabled) { host->bar->Enable(id, enabled); });
	}
}

void buildMenu(wxMenu* menu, std::vector<MenuItem>& items, MenuHost* host, wxFrame* frame)
{
	for (auto& item : items)
		appendItem(menu, item, host, frame);
}

} // unnamed namespace

void attachMenuBar(wxFrame* frame, const MenuBarModel& model)
{
	if (model.empty())
		return;

#ifdef USE_LOGGER
	Logger::instance().log("attachMenuBar()\t-> new wxMenuBar()\n");
#endif
	auto* host = new MenuHost;
	host->model = model; // the caller's Window is a temporary; this copy is ours
	host->bar = new wxMenuBar;

	for (auto& menuModel : host->model.menus)
	{
		auto* menu = new wxMenu;
		buildMenu(menu, menuModel.items, host, frame);
		host->bar->Append(menu, escaped(menuModel.label));
	}

	frame->SetMenuBar(host->bar);
	frame->Bind(wxEVT_DESTROY, [host, frame](wxWindowDestroyEvent& event) {
		if (event.GetWindow() == frame)
			delete host; // wx owns the wxMenuBar itself; this owns the model
		event.Skip();
	});
}
