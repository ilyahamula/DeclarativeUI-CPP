#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Read-only formatted text, parsed ONCE into runs and handed to every backend
// as data -- the same move Shortcut and FileFilter make, for the same reason:
// three backends that each parsed the markup would be three parsers that have
// to agree. The markup subset is
//
//     **bold**   *italic*   [text](url)   {#rrggbb}coloured{/}
//
// and the constructs nest (a bold link, a coloured italic phrase). A backslash
// takes the next character literally, and a newline is a hard line break.
//
// The WRAPPING is shared too. No backend hands this text to a native rich-text
// control: each asks its own fonts how wide a piece of text is and lays the
// runs out with layoutRichText() below, then draws the fragments it returns and
// hit-tests links against them. Wrapping, link areas and the measured height
// therefore follow one rule on all three -- only the glyph widths differ.
struct TextRun
{
	std::string text;
	bool bold = false;
	bool italic = false;
	std::optional<Color> colour; // unset: the backend's text (or link) colour
	std::string link;            // empty: not a link

	bool isLink() const { return !link.empty(); }

	// Same look, same target: two neighbours that agree on all of this are one
	// run, which is what keeps the run list minimal after literal markers fold
	// back into plain text.
	bool sameStyle(const TextRun& other) const
	{
		const bool sameColour = colour.has_value() == other.colour.has_value()
			&& (!colour || (colour->r == other.colour->r && colour->g == other.colour->g
				&& colour->b == other.colour->b && colour->a == other.colour->a));
		return bold == other.bold && italic == other.italic && sameColour && link == other.link;
	}
};

// One piece of a run on one line, positioned relative to the text's top-left.
// `text` is drawn in a single call with the run's style, so `width` is exactly
// what the backend measured for it -- never a sum of word widths.
struct RichTextFragment
{
	std::size_t run = 0;
	std::string text;
	int x = 0;
	int y = 0;
	int width = 0;
};

struct RichTextLayout
{
	std::vector<RichTextFragment> fragments;
	int width = 0;  // the widest line
	int height = 0; // lines x lineHeight; an empty text still takes one line
	int lineHeight = 0;
};

namespace rich_text_detail
{

enum class Tok
{
	Text,
	Bold,        // **
	Italic,      // *
	LinkOpen,    // [
	LinkClose,   // ](url)
	ColourOpen,  // {#rrggbb}
	ColourClose, // {/}
};

struct Token
{
	Tok kind = Tok::Text;
	std::string text;   // the literal spelling, used when the marker is unmatched
	std::string url;    // LinkClose
	Color colour;       // ColourOpen
	bool matched = false;
};

inline int hexDigit(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

// "{#rrggbb}" at `pos`, or nothing. Exactly six digits: a shorter or longer
// spelling is literal text rather than a guess at what was meant.
inline std::optional<Color> colourAt(std::string_view s, std::size_t pos)
{
	if (pos + 9 > s.size() || s[pos] != '{' || s[pos + 1] != '#' || s[pos + 8] != '}')
		return std::nullopt;
	int v[6];
	for (int i = 0; i < 6; ++i)
		if ((v[i] = hexDigit(s[pos + 2 + i])) < 0)
			return std::nullopt;
	return Color { (v[0] * 16 + v[1]) / 255.0f, (v[2] * 16 + v[3]) / 255.0f,
		(v[4] * 16 + v[5]) / 255.0f, 1.0f };
}

inline std::vector<Token> tokenize(std::string_view s)
{
	std::vector<Token> tokens;
	auto literal = [&tokens](std::string_view text) {
		if (!tokens.empty() && tokens.back().kind == Tok::Text)
			tokens.back().text += text;
		else
			tokens.push_back(Token { Tok::Text, std::string(text) });
	};

	for (std::size_t i = 0; i < s.size();)
	{
		const char c = s[i];
		if (c == '\\')
		{
			// A trailing backslash has nothing to escape and is itself.
			literal(i + 1 < s.size() ? s.substr(i + 1, 1) : s.substr(i, 1));
			i += 2;
		}
		else if (c == '*')
		{
			const bool dbl = i + 1 < s.size() && s[i + 1] == '*';
			tokens.push_back(Token { dbl ? Tok::Bold : Tok::Italic, dbl ? "**" : "*" });
			i += dbl ? 2 : 1;
		}
		else if (c == '[')
		{
			tokens.push_back(Token { Tok::LinkOpen, "[" });
			++i;
		}
		else if (c == ']' && i + 1 < s.size() && s[i + 1] == '(')
		{
			// The target runs to the first ')' on the same line. With no ')' the
			// bracket is plain text, and so is everything after it.
			const std::size_t close = s.find_first_of(")\n", i + 2);
			if (close != std::string_view::npos && s[close] == ')' && close > i + 2)
			{
				Token t { Tok::LinkClose, std::string(s.substr(i, close + 1 - i)) };
				t.url = std::string(s.substr(i + 2, close - i - 2));
				tokens.push_back(std::move(t));
				i = close + 1;
			}
			else
			{
				literal(s.substr(i, 1));
				++i;
			}
		}
		else if (c == '{' && s.substr(i, 3) == "{/}")
		{
			tokens.push_back(Token { Tok::ColourClose, "{/}" });
			i += 3;
		}
		else if (const auto colour = c == '{' ? colourAt(s, i) : std::nullopt)
		{
			Token t { Tok::ColourOpen, std::string(s.substr(i, 9)) };
			t.colour = *colour;
			tokens.push_back(std::move(t));
			i += 9;
		}
		else
		{
			literal(s.substr(i, 1));
			++i;
		}
	}
	return tokens;
}

// Pair every closer with an opener. A closer takes the NEAREST open marker of
// its kind; anything opened after that marker and still unclosed is abandoned
// (it will print literally), so markers never cross. `*` and `**` are toggles:
// one closes when its kind is open, and opens otherwise.
inline void matchMarkers(std::vector<Token>& tokens)
{
	std::vector<std::size_t> open;
	auto closeNearest = [&](Tok opener, std::size_t closer) {
		for (std::size_t k = open.size(); k-- > 0;)
		{
			if (tokens[open[k]].kind != opener)
				continue;
			tokens[open[k]].matched = true;
			tokens[closer].matched = true;
			open.resize(k);
			return true;
		}
		return false;
	};

	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		switch (tokens[i].kind)
		{
		case Tok::Bold:
		case Tok::Italic:
			if (!closeNearest(tokens[i].kind, i))
				open.push_back(i);
			break;
		case Tok::LinkOpen:
		case Tok::ColourOpen:
			open.push_back(i);
			break;
		case Tok::LinkClose:
			closeNearest(Tok::LinkOpen, i);
			break;
		case Tok::ColourClose:
			closeNearest(Tok::ColourOpen, i);
			break;
		case Tok::Text:
			break;
		}
	}
}

// Byte length of the UTF-8 sequence starting at `lead`, so a word that has to
// be broken is never split inside a character. A stray continuation byte counts
// as one: malformed input still makes progress.
inline std::size_t utf8Length(unsigned char lead)
{
	if (lead >= 0xF0)
		return 4;
	if (lead >= 0xE0)
		return 3;
	if (lead >= 0xC0)
		return 2;
	return 1;
}

inline bool isSpace(char c)
{
	return c == ' ' || c == '\t';
}

} // namespace rich_text_detail

// Parses the markup above into runs. Never throws, and never half-honours a
// construct: a marker with no partner is shown as the text it was written as,
// so malformed markup degrades to what the caller typed. Empty runs are
// dropped and neighbours with the same style are merged.
inline std::vector<TextRun> parseMarkup(std::string_view markup)
{
	using namespace rich_text_detail;
	std::vector<Token> tokens = tokenize(markup);
	matchMarkers(tokens);

	std::vector<TextRun> runs;
	int bold = 0;
	int italic = 0;
	std::vector<Color> colours;
	std::vector<std::string> links;

	auto emit = [&](const std::string& text) {
		if (text.empty())
			return;
		TextRun run;
		run.text = text;
		run.bold = bold > 0;
		run.italic = italic > 0;
		if (!colours.empty())
			run.colour = colours.back();
		if (!links.empty())
			run.link = links.back();
		if (!runs.empty() && runs.back().sameStyle(run))
			runs.back().text += text;
		else
			runs.push_back(std::move(run));
	};

	// A link's URL sits on its CLOSER, so the opener has to look ahead for it.
	auto urlFor = [&tokens](std::size_t opener) {
		int depth = 0;
		for (std::size_t k = opener + 1; k < tokens.size(); ++k)
		{
			if (!tokens[k].matched)
				continue;
			if (tokens[k].kind == Tok::LinkOpen)
				++depth;
			else if (tokens[k].kind == Tok::LinkClose && depth-- == 0)
				return tokens[k].url;
		}
		return std::string();
	};

	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		const Token& t = tokens[i];
		if (t.kind == Tok::Text || !t.matched)
		{
			emit(t.text);
			continue;
		}
		switch (t.kind)
		{
		case Tok::Bold:
			bold = bold > 0 ? 0 : 1;
			break;
		case Tok::Italic:
			italic = italic > 0 ? 0 : 1;
			break;
		case Tok::LinkOpen:
			links.push_back(urlFor(i));
			break;
		case Tok::LinkClose:
			links.pop_back();
			break;
		case Tok::ColourOpen:
			colours.push_back(t.colour);
			break;
		case Tok::ColourClose:
			colours.pop_back();
			break;
		case Tok::Text:
			break;
		}
	}
	return runs;
}

// Lays `runs` out in lines no wider than `maxWidth` (<= 0: no limit), each
// `lineHeight` tall. `measure(run, text)` is the backend's width of `text` in
// that run's style; it is asked about whole candidate fragments rather than
// single words, so kerning and rounding inside a fragment are the font's own.
//
// Breaks fall at spaces, and at run boundaries; a word wider than the whole
// line is broken between characters instead of overflowing. Spaces that a
// break falls on are dropped, and a newline in a run always breaks.
template <typename MeasureFn>
RichTextLayout layoutRichText(const std::vector<TextRun>& runs, int maxWidth, int lineHeight,
	MeasureFn&& measure)
{
	using namespace rich_text_detail;
	RichTextLayout layout;
	layout.lineHeight = lineHeight;

	int line = 0;
	int x = 0; // where the fragment being built starts on the current line
	auto fits = [maxWidth](int right) { return maxWidth <= 0 || right <= maxWidth; };

	for (std::size_t r = 0; r < runs.size(); ++r)
	{
		const TextRun& run = runs[r];
		std::string frag;
		std::string pendingSpace;

		auto flush = [&](bool withSpace) {
			std::string text = withSpace ? frag + pendingSpace : frag;
			if (!text.empty())
			{
				const int w = measure(run, std::string_view(text));
				layout.fragments.push_back(RichTextFragment { r, text, x, line * lineHeight, w });
				x += w;
				layout.width = std::max(layout.width, x);
			}
			frag.clear();
			pendingSpace.clear();
		};
		auto newLine = [&] {
			++line;
			x = 0;
		};
		// A word that cannot fit even on a line of its own, placed at the start
		// of one: peel off the longest prefix that fits (one character at least,
		// so a hopelessly narrow frame still terminates) until the rest does.
		auto placeLongWord = [&](std::string word) {
			while (!fits(measure(run, std::string_view(word))))
			{
				std::size_t cut = 0;
				for (std::size_t next = 0; next < word.size();)
				{
					next += utf8Length((unsigned char)word[next]);
					next = std::min(next, word.size());
					if (cut > 0 && !fits(measure(run, std::string_view(word).substr(0, next))))
						break;
					cut = next;
				}
				if (cut >= word.size())
					break;
				frag = word.substr(0, cut);
				flush(false);
				newLine();
				word.erase(0, cut);
			}
			frag = std::move(word);
		};

		const std::string& text = run.text;
		for (std::size_t i = 0; i < text.size();)
		{
			if (text[i] == '\n')
			{
				flush(false);
				newLine();
				++i;
				continue;
			}
			if (isSpace(text[i]))
			{
				std::size_t end = i;
				while (end < text.size() && isSpace(text[end]))
					++end;
				// Space at the start of a line is dropped; anywhere else it is
				// held until the next word shows whether the line goes on.
				if (x > 0 || !frag.empty())
					pendingSpace += text.substr(i, end - i);
				i = end;
				continue;
			}

			std::size_t end = i;
			while (end < text.size() && text[end] != '\n' && !isSpace(text[end]))
				++end;
			const std::string word = text.substr(i, end - i);
			i = end;

			const std::string candidate = frag + pendingSpace + word;
			if (fits(x + measure(run, std::string_view(candidate))))
			{
				frag = candidate;
				pendingSpace.clear();
				continue;
			}
			// The word does not fit after what this line already holds: close the
			// line there (dropping the space the break fell on) and start the
			// word on the next one -- unless the line is still empty, in which
			// case no break would help and the word is broken instead.
			if (x > 0 || !frag.empty())
			{
				flush(false);
				newLine();
			}
			pendingSpace.clear();
			placeLongWord(word);
		}
		// Trailing space belongs to this run's style and may separate it from
		// the next run on the same line, so it is kept.
		flush(true);
	}

	layout.height = (line + 1) * lineHeight;
	return layout;
}

// The URL under (px, py) -- a point relative to the text's top-left -- or
// nullptr. Hit areas are the fragments' own rectangles, one line tall.
inline const std::string* linkAt(const RichTextLayout& layout, const std::vector<TextRun>& runs,
	int px, int py)
{
	for (const RichTextFragment& f : layout.fragments)
	{
		if (f.run >= runs.size() || !runs[f.run].isLink())
			continue;
		if (px >= f.x && px < f.x + f.width && py >= f.y && py < f.y + layout.lineHeight)
			return &runs[f.run].link;
	}
	return nullptr;
}
