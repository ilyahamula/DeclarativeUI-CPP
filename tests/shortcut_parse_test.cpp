#include "test_framework.hpp"

#include "frameworks_core/CoreTypes/Shortcut.hpp"

// Backend-free, like every test here: Shortcut is plain data in CoreTypes, and
// each backend maps its FIELDS to a native accelerator through its own table.
// What is tested here is the one thing all three share -- the parse.

namespace
{

Shortcut parsed(std::string_view text)
{
	const std::optional<Shortcut> result = Shortcut::parse(text);
	CHECK(result.has_value());
	return result.value_or(Shortcut{});
}

} // unnamed namespace

TEST(shortcut_parses_modifiers_and_key)
{
	const Shortcut s = parsed("Ctrl+Shift+S");
	CHECK(s.ctrl);
	CHECK(s.shift);
	CHECK(!s.alt);
	CHECK(!s.super);
	CHECK_EQ(s.key, std::string("S"));
}

TEST(shortcut_bare_key_has_no_modifiers)
{
	const Shortcut s = parsed("F5");
	CHECK(!s.ctrl);
	CHECK(!s.shift);
	CHECK(!s.alt);
	CHECK_EQ(s.key, std::string("F5"));
}

TEST(shortcut_names_are_case_insensitive)
{
	CHECK(Shortcut::parse("ctrl+s") == Shortcut::parse("CTRL+S"));
	CHECK(Shortcut::parse("CtRl+S") == Shortcut::parse("Ctrl+s"));
	// The key canonicalises to upper case whichever way it was written.
	CHECK_EQ(parsed("ctrl+s").key, std::string("S"));
}

TEST(shortcut_ignores_surrounding_space)
{
	CHECK(Shortcut::parse(" Ctrl + Shift + S ") == Shortcut::parse("Ctrl+Shift+S"));
}

TEST(shortcut_modifier_aliases)
{
	CHECK(parsed("Control+A").ctrl);
	CHECK(parsed("Option+A").alt);
	// Cmd/Win/Meta all mean the same explicit super key, which is display-only:
	// "Ctrl" is what a caller writes to get Cmd on macOS.
	CHECK(parsed("Cmd+A").super);
	CHECK(parsed("Command+A").super);
	CHECK(parsed("Win+A").super);
	CHECK(parsed("Meta+A").super);
	CHECK(parsed("Super+A").super);
	CHECK(!parsed("Cmd+A").ctrl);
}

TEST(shortcut_key_aliases_canonicalise)
{
	CHECK_EQ(parsed("Esc").key, std::string("Escape"));
	CHECK_EQ(parsed("Escape").key, std::string("Escape"));
	CHECK_EQ(parsed("Del").key, std::string("Delete"));
	CHECK_EQ(parsed("Ins").key, std::string("Insert"));
	CHECK_EQ(parsed("PgUp").key, std::string("PageUp"));
	CHECK_EQ(parsed("PageDown").key, std::string("PageDown"));
	CHECK_EQ(parsed("Return").key, std::string("Enter"));
	CHECK_EQ(parsed("Back").key, std::string("Backspace"));
	CHECK_EQ(parsed("ctrl+left").key, std::string("Left"));
	CHECK_EQ(parsed("7").key, std::string("7"));
}

TEST(shortcut_function_key_range)
{
	CHECK_EQ(parsed("F1").key, std::string("F1"));
	CHECK_EQ(parsed("F12").key, std::string("F12"));
	CHECK(!Shortcut::parse("F0").has_value());
	CHECK(!Shortcut::parse("F13").has_value());
	CHECK(!Shortcut::parse("F1x").has_value());
}

TEST(shortcut_display_round_trips)
{
	CHECK_EQ(parsed("Ctrl+Shift+S").display(), std::string("Ctrl+Shift+S"));
	CHECK_EQ(parsed("F5").display(), std::string("F5"));
	CHECK_EQ(parsed("alt+left").display(), std::string("Alt+Left"));
	// Modifiers come out in the order every desktop platform writes them,
	// whatever order they went in.
	CHECK_EQ(parsed("Shift+Alt+Ctrl+S").display(), std::string("Ctrl+Alt+Shift+S"));
	CHECK_EQ(parsed("Cmd+S").display(), std::string("Meta+S"));
	// Re-parsing what we display is the same shortcut.
	CHECK(Shortcut::parse(parsed("Shift+Alt+Ctrl+S").display()) == parsed("Ctrl+Alt+Shift+S"));
}

TEST(shortcut_default_displays_as_nothing)
{
	CHECK_EQ(Shortcut{}.display(), std::string());
}

TEST(shortcut_malformed_input_yields_nullopt)
{
	// Malformed text never throws and never half-parses: the item simply gets
	// no accelerator (the backends log why).
	CHECK(!Shortcut::parse("").has_value());
	CHECK(!Shortcut::parse("   ").has_value());
	CHECK(!Shortcut::parse("Ctrl").has_value());       // modifiers alone
	CHECK(!Shortcut::parse("Ctrl+Shift").has_value());
	CHECK(!Shortcut::parse("Ctrl+").has_value());      // trailing separator
	CHECK(!Shortcut::parse("+S").has_value());
	CHECK(!Shortcut::parse("Ctrl++S").has_value());    // empty token
	CHECK(!Shortcut::parse("Ctrl+S+T").has_value());   // two keys
	CHECK(!Shortcut::parse("Ctrl+Banana").has_value());// a key we cannot name
}
