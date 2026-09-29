#include "test_framework.hpp"

#include "frameworks_core/CoreTypes/RichTextRuns.hpp"

#include <string>
#include <string_view>
#include <vector>

// Backend-free, like every test here. RichText is parsed once into runs and laid
// out by one shared routine; each backend only supplies its font's widths. The
// layout tests use a monospace measurer (10 px a byte, bold one wider), which
// is enough to pin every wrapping rule without a font.

namespace
{

int mono(const TextRun& run, std::string_view text)
{
	return (int)text.size() * (run.bold ? 11 : 10);
}

RichTextLayout monoLayout(const std::vector<TextRun>& runs, int maxWidth)
{
	return layoutRichText(runs, maxWidth, 20, mono);
}

// The lines as strings, fragments on one line joined -- what a reader would see.
std::vector<std::string> lines(const RichTextLayout& layout)
{
	std::vector<std::string> out;
	for (const RichTextFragment& f : layout.fragments)
	{
		const std::size_t line = (std::size_t)(f.y / layout.lineHeight);
		if (out.size() <= line)
			out.resize(line + 1);
		out[line] += f.text;
	}
	return out;
}

// By value: CHECK_EQ binds its operands by reference, and a member of the
// temporary vector would not outlive the full expression that binds it.
std::string firstText(std::string_view markup)
{
	const std::vector<TextRun> runs = parseMarkup(markup);
	return runs.empty() ? std::string() : runs[0].text;
}

} // namespace

TEST(richtext_plain_text_is_one_run)
{
	const std::vector<TextRun> runs = parseMarkup("Hello, world");
	CHECK_EQ(runs.size(), (std::size_t)1);
	CHECK_EQ(runs[0].text, std::string("Hello, world"));
	CHECK(!runs[0].bold && !runs[0].italic && !runs[0].colour && !runs[0].isLink());
}

TEST(richtext_bold_and_italic)
{
	const std::vector<TextRun> runs = parseMarkup("a **b** *c* d");
	CHECK_EQ(runs.size(), (std::size_t)5);
	CHECK_EQ(runs[1].text, std::string("b"));
	CHECK(runs[1].bold && !runs[1].italic);
	CHECK_EQ(runs[3].text, std::string("c"));
	CHECK(runs[3].italic && !runs[3].bold);
	CHECK_EQ(runs[4].text, std::string(" d"));
}

TEST(richtext_link_carries_its_url)
{
	const std::vector<TextRun> runs = parseMarkup("see [the docs](https://example.com/a?b=c) now");
	CHECK_EQ(runs.size(), (std::size_t)3);
	CHECK_EQ(runs[1].text, std::string("the docs"));
	CHECK_EQ(runs[1].link, std::string("https://example.com/a?b=c"));
	CHECK(!runs[0].isLink() && !runs[2].isLink());
}

TEST(richtext_colour_span)
{
	const std::vector<TextRun> runs = parseMarkup("{#ff8000}warm{/} cold");
	CHECK_EQ(runs.size(), (std::size_t)2);
	CHECK(runs[0].colour.has_value());
	CHECK(runs[0].colour->r == 1.0f);
	CHECK(runs[0].colour->g > 0.49f && runs[0].colour->g < 0.51f);
	CHECK(runs[0].colour->b == 0.0f);
	CHECK(!runs[1].colour);
}

TEST(richtext_constructs_nest)
{
	const std::vector<TextRun> runs = parseMarkup("**bold [link *both*](u)** {#00ff00}g {#0000ff}b{/} g{/}");
	CHECK_EQ(runs[0].text, std::string("bold "));
	CHECK(runs[0].bold && !runs[0].isLink());
	CHECK_EQ(runs[1].text, std::string("link "));
	CHECK(runs[1].bold && runs[1].link == "u");
	CHECK_EQ(runs[2].text, std::string("both"));
	CHECK(runs[2].bold && runs[2].italic && runs[2].link == "u");
	// Colours restore the outer one when an inner span closes.
	const TextRun& inner = runs[runs.size() - 2];
	const TextRun& outer = runs.back();
	CHECK_EQ(inner.text, std::string("b"));
	CHECK(inner.colour && inner.colour->b == 1.0f);
	CHECK_EQ(outer.text, std::string(" g"));
	CHECK(outer.colour && outer.colour->g == 1.0f);
}

TEST(richtext_backslash_escapes)
{
	const std::vector<TextRun> runs = parseMarkup("2\\*3\\*4 \\[x\\] back\\\\slash end\\");
	CHECK_EQ(runs.size(), (std::size_t)1);
	CHECK_EQ(runs[0].text, std::string("2*3*4 [x] back\\slash end\\"));
}

TEST(richtext_unmatched_markers_print_literally)
{
	CHECK_EQ(firstText("**open"), std::string("**open"));
	CHECK_EQ(firstText("a * b"), std::string("a * b"));
	CHECK_EQ(firstText("[no target]"), std::string("[no target]"));
	CHECK_EQ(firstText("[a](unclosed"), std::string("[a](unclosed"));
	CHECK_EQ(firstText("x](u) y"), std::string("x](u) y"));
	CHECK_EQ(firstText("{#12345}x{/}"), std::string("{#12345}x{/}"));
	CHECK_EQ(firstText("{#gg0000}x"), std::string("{#gg0000}x"));
	CHECK_EQ(firstText("{/} stray"), std::string("{/} stray"));
}

TEST(richtext_markers_never_cross)
{
	// The link closes over an italic opened inside it and never closed: that
	// '*' prints, and the '*' after the link opens nothing that closes either.
	const std::vector<TextRun> runs = parseMarkup("[a *b](u) c*");
	CHECK_EQ(runs.size(), (std::size_t)2);
	CHECK_EQ(runs[0].text, std::string("a *b"));
	CHECK(runs[0].link == "u" && !runs[0].italic);
	CHECK_EQ(runs[1].text, std::string(" c*"));
	CHECK(!runs[1].italic);
}

TEST(richtext_empty_and_marker_only_input)
{
	CHECK(parseMarkup("").empty());
	CHECK(parseMarkup("****").empty());
	CHECK(parseMarkup("[](u)").empty());
}

TEST(richtext_same_style_neighbours_merge)
{
	// An unmatched marker folds back into the text around it as one run.
	const std::vector<TextRun> runs = parseMarkup("a [ b");
	CHECK_EQ(runs.size(), (std::size_t)1);
	CHECK_EQ(runs[0].text, std::string("a [ b"));
}

TEST(richtext_layout_unbounded_is_one_line)
{
	const RichTextLayout layout = monoLayout(parseMarkup("one **two** three"), 0);
	CHECK_EQ(lines(layout).size(), (std::size_t)1);
	CHECK_EQ(layout.height, 20);
	// "one " + "two" in bold + " three"
	CHECK_EQ(layout.width, 40 + 33 + 60);
}

TEST(richtext_layout_wraps_at_spaces)
{
	const RichTextLayout layout = monoLayout(parseMarkup("aaa bbb ccc ddd"), 75);
	const std::vector<std::string> got = lines(layout);
	CHECK_EQ(got.size(), (std::size_t)2);
	CHECK_EQ(got[0], std::string("aaa bbb"));
	CHECK_EQ(got[1], std::string("ccc ddd"));
	CHECK_EQ(layout.height, 40);
	CHECK(layout.width <= 75);
}

TEST(richtext_layout_wraps_across_runs)
{
	// The break falls on the space that begins the plain run, which is dropped,
	// and the wrapped line starts at x = 0.
	const RichTextLayout layout = monoLayout(parseMarkup("**bold** plain"), 60);
	CHECK_EQ(lines(layout).size(), (std::size_t)2);
	CHECK_EQ(std::string(lines(layout)[1]), std::string("plain"));
	const RichTextFragment& last = layout.fragments.back();
	CHECK_EQ(last.x, 0);
	CHECK_EQ(last.y, 20);
}

TEST(richtext_layout_fragments_follow_each_other)
{
	const std::vector<TextRun> runs = parseMarkup("ab**cd**ef");
	const RichTextLayout layout = monoLayout(runs, 0);
	CHECK_EQ(layout.fragments.size(), (std::size_t)3);
	CHECK_EQ(layout.fragments[1].x, 20);
	CHECK_EQ(layout.fragments[1].width, 22);
	CHECK_EQ(layout.fragments[2].x, 42);
}

TEST(richtext_layout_breaks_a_word_wider_than_the_line)
{
	const RichTextLayout layout = monoLayout(parseMarkup("abcdefghij"), 35);
	const std::vector<std::string> got = lines(layout);
	CHECK_EQ(got.size(), (std::size_t)4);
	CHECK_EQ(got[0], std::string("abc"));
	CHECK_EQ(got[3], std::string("j"));
}

TEST(richtext_layout_never_splits_a_utf8_character)
{
	// Three two-byte characters, 20 px each in the monospace measurer.
	const RichTextLayout layout = monoLayout(parseMarkup("\xC3\xA9\xC3\xA9\xC3\xA9"), 30);
	for (const RichTextFragment& f : layout.fragments)
		CHECK_EQ(f.text, std::string("\xC3\xA9"));
}

TEST(richtext_layout_hopelessly_narrow_still_terminates)
{
	const RichTextLayout layout = monoLayout(parseMarkup("abc"), 1);
	CHECK_EQ(lines(layout).size(), (std::size_t)3);
}

TEST(richtext_layout_newline_is_a_hard_break)
{
	const RichTextLayout layout = monoLayout(parseMarkup("a\n\nb"), 0);
	CHECK_EQ(layout.height, 60);
	CHECK_EQ(layout.fragments.back().y, 40);
}

TEST(richtext_layout_empty_text_is_one_line)
{
	const RichTextLayout layout = monoLayout({}, 100);
	CHECK_EQ(layout.height, 20);
	CHECK_EQ(layout.width, 0);
}

TEST(richtext_link_hit_test)
{
	const std::vector<TextRun> runs = parseMarkup("go [here](u1) or [there](u2)");
	const RichTextLayout layout = monoLayout(runs, 0);
	// "go " is 30 px; "here" runs 30..70; " or " to 110; "there" 110..160.
	CHECK(linkAt(layout, runs, 10, 5) == nullptr);
	const std::string* first = linkAt(layout, runs, 30, 5);
	CHECK(first != nullptr && *first == "u1");
	CHECK(linkAt(layout, runs, 69, 19) != nullptr);
	CHECK(linkAt(layout, runs, 70, 5) == nullptr);
	const std::string* second = linkAt(layout, runs, 150, 10);
	CHECK(second != nullptr && *second == "u2");
	CHECK(linkAt(layout, runs, 150, 20) == nullptr);
}
