#include "frameworks_core/imgui/MenuDraw.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include "imgui.h"

#include <string>
#include <unordered_map>

namespace
{

// An UNBOUND check item has no home in the model: the caller rebuilds the menu
// every frame, so the literal it was built from would win before every draw and
// a snapshot check item could never be ticked. It lives here instead, keyed by
// the item's path through the bar ("View/Word wrap") -- the same move
// imgui/WindowWrapper.cpp makes for a window's close state, keyed by the one
// thing about the item that is stable across a rebuild.
//
// Bound items need none of this: they ARE the caller's bool.
std::unordered_map<std::string, bool>& snapshotChecks()
{
	static std::unordered_map<std::string, bool> checks;
	return checks;
}

bool checkedState(const MenuItem& item, const std::string& key)
{
	if (!item.checkedFlag)
		return false;
	if (item.checkedFlag->isBound())
		return item.checkedFlag->get();

	auto& store = snapshotChecks();
	const auto found = store.find(key);
	if (found != store.end())
		return found->second;
	// First sight of this item: seed the store from the caller's literal.
	const bool seed = item.checkedFlag->get();
	store[key] = seed;
	return seed;
}

// The one place the model is written through. A BOUND flag writes to the
// caller's own bool, which the model merely points at and never owns -- so the
// const here is about our handle on the item, not about the value behind it.
// An unbound flag never touches the model at all.
void writeChecked(const MenuItem& item, const std::string& key, bool value)
{
	if (!item.checkedFlag)
		return;
	if (item.checkedFlag->isBound())
		const_cast<MenuItem&>(item).checkedFlag->set(value);
	else
		snapshotChecks()[key] = value;
}

// ImGui's own key table -- the third spelling of the same shortcut, and the
// reason Shortcut carries parsed fields rather than a string.
//
// ImGuiMod_Ctrl is "Ctrl (non-macOS), Cmd (macOS)" and needs no help from us:
// the swap happens inside ImGuiIO::AddKeyEvent, which exchanges Cmd and Ctrl
// under io.ConfigMacOSXBehaviors (on by default on __APPLE__) before the key
// ever reaches a chord. The GLFW backend's own mapping is platform-neutral --
// physical Cmd arrives as ImGuiMod_Super -- and ImGui turns that into
// ImGuiMod_Ctrl on the way in. So "Ctrl+S" is Cmd+S here exactly as it is on
// wx and Qt, with no special case, which is what the headless probe checks.
//
// `super` is absent here as it is on the other two backends: display-only.
bool chordFor(const Shortcut& shortcut, ImGuiKeyChord& out)
{
	ImGuiKey key = ImGuiKey_None;
	const std::string& name = shortcut.key;

	if (name.size() == 1)
	{
		const char c = name.front();
		if (c >= 'A' && c <= 'Z')
			key = (ImGuiKey)(ImGuiKey_A + (c - 'A'));
		else if (c >= '0' && c <= '9')
			key = (ImGuiKey)(ImGuiKey_0 + (c - '0'));
	}
	else if (name.size() >= 2 && name[0] == 'F')
	{
		const int number = std::stoi(name.substr(1));
		if (number >= 1 && number <= 12)
			key = (ImGuiKey)(ImGuiKey_F1 + (number - 1));
	}
	else if (name == "Enter")     key = ImGuiKey_Enter;
	else if (name == "Escape")    key = ImGuiKey_Escape;
	else if (name == "Delete")    key = ImGuiKey_Delete;
	else if (name == "Insert")    key = ImGuiKey_Insert;
	else if (name == "Home")      key = ImGuiKey_Home;
	else if (name == "End")       key = ImGuiKey_End;
	else if (name == "PageUp")    key = ImGuiKey_PageUp;
	else if (name == "PageDown")  key = ImGuiKey_PageDown;
	else if (name == "Tab")       key = ImGuiKey_Tab;
	else if (name == "Space")     key = ImGuiKey_Space;
	else if (name == "Backspace") key = ImGuiKey_Backspace;
	else if (name == "Left")      key = ImGuiKey_LeftArrow;
	else if (name == "Right")     key = ImGuiKey_RightArrow;
	else if (name == "Up")        key = ImGuiKey_UpArrow;
	else if (name == "Down")      key = ImGuiKey_DownArrow;
	else if (name == "Plus")      key = ImGuiKey_Equal; // '+' is shift+'=' on the layouts ImGui reports
	else if (name == "Minus")     key = ImGuiKey_Minus;
	else if (name == "Comma")     key = ImGuiKey_Comma;
	else if (name == "Period")    key = ImGuiKey_Period;
	else if (name == "Slash")     key = ImGuiKey_Slash;

	if (key == ImGuiKey_None)
		return false;

	ImGuiKeyChord chord = (ImGuiKeyChord)key;
	if (shortcut.ctrl)
		chord |= ImGuiMod_Ctrl;
	if (shortcut.alt)
		chord |= ImGuiMod_Alt;
	if (shortcut.shift)
		chord |= ImGuiMod_Shift;
	out = chord;
	return true;
}

// Value first, then the callback -- the rule every control in the framework
// follows, so a handler reading the bound bool sees the state just selected.
void activate(const MenuItem& item, const std::string& key, bool checked)
{
	writeChecked(item, key, checked);
	if (item.selectHandler)
		item.selectHandler();
}

// What the item shows in its shortcut column. The chord already fires on Cmd
// under Mac conventions (see chordFor); this is only about reading right --
// wx and Qt render a Ctrl accelerator the way the platform spells it, while
// ImGui draws exactly the string it is handed.
std::string shortcutLabel(const MenuItem& item)
{
	if (!item.shortcut || !ImGui::GetIO().ConfigMacOSXBehaviors || !item.shortcut->ctrl)
		return item.shortcutText;
	std::string text = item.shortcutText;
	const std::size_t at = text.find("Ctrl+");
	if (at != std::string::npos)
		text.replace(at, 5, "Cmd+");
	return text;
}

std::string childPath(const std::string& path, const MenuItem& item)
{
	return path.empty() ? item.label : path + "/" + item.label;
}

} // unnamed namespace

void drawMenuItems(const std::vector<MenuItem>& items, const std::string& path)
{
	for (std::size_t index = 0; index < items.size(); ++index)
	{
		const MenuItem& item = items[index];
		if (item.isSeparator)
		{
			// Inside a menu popup the popup IS the window, so Separator()
			// spanning it is exactly what a menu separator should do.
			ImGui::Separator();
			continue;
		}

		// Two items may legitimately share a label in one menu; the index is
		// what keeps their ImGui ids apart.
		ImGui::PushID((int)index);
		const bool enabled = !item.disabledFlag.get();
		if (!item.submenu.empty())
		{
			if (ImGui::BeginMenu(item.label.c_str(), enabled))
			{
				drawMenuItems(item.submenu, childPath(path, item));
				ImGui::EndMenu();
			}
		}
		else
		{
			const std::string key = childPath(path, item);
			const std::string label = shortcutLabel(item);
			const char* shortcutText = label.empty() ? nullptr : label.c_str();
			if (item.checkedFlag)
			{
				bool checked = checkedState(item, key);
				if (ImGui::MenuItem(item.label.c_str(), shortcutText, &checked, enabled))
					activate(item, key, checked);
			}
			else if (ImGui::MenuItem(item.label.c_str(), shortcutText, false, enabled))
			{
				activate(item, key, false);
			}
		}
		ImGui::PopID();
	}
}

void routeMenuShortcuts(const std::vector<MenuItem>& items, const std::string& path)
{
	for (const MenuItem& item : items)
	{
		if (item.isSeparator)
			continue;
		if (!item.submenu.empty())
		{
			// A submenu's own item never selects, but its children's shortcuts
			// work whether or not anyone has opened it.
			routeMenuShortcuts(item.submenu, childPath(path, item));
			continue;
		}
		ImGuiKeyChord chord = 0;
		if (!item.shortcut || !chordFor(*item.shortcut, chord))
		{
#ifdef USE_LOGGER
			if (!item.shortcutText.empty() && !item.shortcut)
				Logger::instance().log("drawMenuBar()\t-> unparseable shortcut \""
					+ item.shortcutText + "\" on \"" + item.label + "\": no accelerator registered\n");
#endif
			continue;
		}

		// Polled even while the item is disabled, and the HIT is dropped instead.
		// ImGui owns a shortcut's route only from the frame after it is first
		// polled, so skipping the call outright would cost the item a frame of
		// deafness every time it came back -- press Ctrl+Z on the frame the
		// undo stack refills and nothing would happen.
		const bool fired = ImGui::Shortcut(chord, ImGuiInputFlags_RouteFocused);
		if (fired && !item.disabledFlag.get())
		{
			const std::string key = childPath(path, item);
			activate(item, key, item.checkedFlag ? !checkedState(item, key) : false);
		}
	}
}

void drawMenuBar(const MenuBarModel& model)
{
	if (model.empty())
		return;

	if (ImGui::BeginMenuBar())
	{
		for (std::size_t index = 0; index < model.menus.size(); ++index)
		{
			const Menu& menu = model.menus[index];
			ImGui::PushID((int)index);
			if (ImGui::BeginMenu(menu.label.c_str()))
			{
				drawMenuItems(menu.items, menu.label);
				ImGui::EndMenu();
			}
			ImGui::PopID();
		}
		ImGui::EndMenuBar();
	}

	// Outside the bar: a shortcut fires whether or not its menu was opened.
	for (const Menu& menu : model.menus)
		routeMenuShortcuts(menu.items, menu.label);
}
