#pragma once

#include <cctype>
#include <optional>
#include <string>
#include <string_view>

// A keyboard accelerator, parsed once from caller text such as "Ctrl+Shift+S"
// and handed to every backend as data. Each backend maps the FIELDS to its own
// native accelerator (a wx accelerator-table entry, a QKeySequence, an
// ImGuiKeyChord) through its own key table rather than re-parsing a string --
// the three disagree on how keys are spelled, and only the backend knows its
// own spelling. display() is the one string the user ever sees.
//
// Ctrl means Cmd on macOS on all three backends and nothing had to be done to
// make that true: wx and Qt remap Ctrl->Cmd themselves, and ImGuiMod_Ctrl is
// documented as "Ctrl (non-macOS), Cmd (macOS)". One shortcut string therefore
// reads native on every platform, which is the point.
//
// `super` is the escape hatch for an explicitly asked-for Win/Cmd key. It is
// parsed and displayed but NEVER registered: wx has no portable spelling for
// it, and a modifier that fires on one backend only would break the promise
// above. Write "Ctrl" and let the platform decide.
struct Shortcut
{
	bool ctrl = false;
	bool shift = false;
	bool alt = false;
	bool super = false;
	std::string key; // canonical: "S", "5", "F11", "Enter", "Left"

	bool operator==(const Shortcut&) const = default;

	// Parses "Ctrl+Shift+S", "F5", "Alt+Left". Modifier and key names are
	// case-insensitive and surrounding spaces are ignored. Returns nullopt for
	// anything it cannot name a key in -- an empty string, modifiers with no
	// key, a key it does not know -- and never throws: the caller's text
	// arrives from a literal that nothing validated.
	static std::optional<Shortcut> parse(std::string_view text);

	// The human spelling, in the order every desktop platform writes it.
	// Empty when `key` is (a default-constructed Shortcut displays as nothing).
	std::string display() const;
};

namespace shortcut_detail
{

inline std::string lowered(std::string_view text)
{
	std::string out;
	out.reserve(text.size());
	for (const char c : text)
		out.push_back((char)std::tolower((unsigned char)c));
	return out;
}

inline std::string_view trimmed(std::string_view text)
{
	while (!text.empty() && std::isspace((unsigned char)text.front()))
		text.remove_prefix(1);
	while (!text.empty() && std::isspace((unsigned char)text.back()))
		text.remove_suffix(1);
	return text;
}

// The canonical spelling of a key name, or empty for one we cannot name.
// Aliases exist because callers write what their platform taught them ("Esc",
// "Del", "PgUp"); the canonical form is what the backends' tables key on.
inline std::string canonicalKey(std::string_view token)
{
	if (token.size() == 1)
	{
		const unsigned char c = (unsigned char)token.front();
		if (std::isalpha(c))
			return std::string(1, (char)std::toupper(c));
		if (std::isdigit(c))
			return std::string(1, (char)c);
		return {};
	}

	const std::string name = lowered(token);

	// F1-F12
	if (name.size() >= 2 && name[0] == 'f' && std::isdigit((unsigned char)name[1]))
	{
		const std::string digits = name.substr(1);
		if (digits.find_first_not_of("0123456789") != std::string::npos)
			return {};
		const int number = std::stoi(digits);
		if (number >= 1 && number <= 12)
			return "F" + std::to_string(number);
		return {};
	}

	if (name == "enter" || name == "return")       return "Enter";
	if (name == "esc" || name == "escape")         return "Escape";
	if (name == "del" || name == "delete")         return "Delete";
	if (name == "ins" || name == "insert")         return "Insert";
	if (name == "home")                            return "Home";
	if (name == "end")                             return "End";
	if (name == "pgup" || name == "pageup")        return "PageUp";
	if (name == "pgdn" || name == "pagedown")      return "PageDown";
	if (name == "tab")                             return "Tab";
	if (name == "space")                           return "Space";
	if (name == "back" || name == "backspace")     return "Backspace";
	if (name == "left")                            return "Left";
	if (name == "right")                           return "Right";
	if (name == "up")                              return "Up";
	if (name == "down")                            return "Down";
	if (name == "plus")                            return "Plus";
	if (name == "minus")                           return "Minus";
	if (name == "comma")                           return "Comma";
	if (name == "period" || name == "dot")         return "Period";
	if (name == "slash")                           return "Slash";
	return {};
}

} // namespace shortcut_detail

inline std::optional<Shortcut> Shortcut::parse(std::string_view text)
{
	Shortcut result;
	bool haveKey = false;

	std::string_view rest = text;
	while (true)
	{
		const std::size_t plus = rest.find('+');
		const std::string_view token = shortcut_detail::trimmed(
			plus == std::string_view::npos ? rest : rest.substr(0, plus));
		if (token.empty())
			return std::nullopt; // "Ctrl+", "++", " " -- nothing to name

		// The key is whatever is not a modifier, and there is exactly one:
		// "Ctrl+S+S" is as malformed as "Ctrl+".
		const std::string name = shortcut_detail::lowered(token);
		if (name == "ctrl" || name == "control")
			result.ctrl = true;
		else if (name == "shift")
			result.shift = true;
		else if (name == "alt" || name == "option")
			result.alt = true;
		else if (name == "cmd" || name == "command" || name == "super"
			|| name == "win" || name == "meta")
			result.super = true;
		else
		{
			if (haveKey)
				return std::nullopt;
			result.key = shortcut_detail::canonicalKey(token);
			if (result.key.empty())
				return std::nullopt;
			haveKey = true;
		}

		if (plus == std::string_view::npos)
			break;
		rest = rest.substr(plus + 1);
	}

	if (!haveKey)
		return std::nullopt; // modifiers alone are not a shortcut
	return result;
}

inline std::string Shortcut::display() const
{
	if (key.empty())
		return {};
	std::string out;
	if (ctrl)
		out += "Ctrl+";
	if (alt)
		out += "Alt+";
	if (shift)
		out += "Shift+";
	if (super)
		out += "Meta+";
	out += key;
	return out;
}
