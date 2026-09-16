#include "test_framework.hpp"

#include "frameworks_core/CoreTypes/FileFilter.hpp"

#include <string>

// Backend-free, like every test here: FileFilter is plain data in CoreTypes and
// each backend rebuilds its OWN wildcard string from the parsed fields. What is
// tested here is the one thing all three share -- the parse, and the glob the
// ImGui browser filters with.

TEST(file_filter_parses_one_group)
{
	const std::vector<FileFilter> filters = FileFilter::parse("Images (*.png;*.jpg)|*.png;*.jpg");
	CHECK_EQ(filters.size(), (std::size_t)1);
	CHECK_EQ(filters[0].description, std::string("Images (*.png;*.jpg)"));
	CHECK_EQ(filters[0].patterns.size(), (std::size_t)2);
	CHECK_EQ(filters[0].patterns[0], std::string("*.png"));
	CHECK_EQ(filters[0].patterns[1], std::string("*.jpg"));
}

TEST(file_filter_parses_several_groups)
{
	const std::vector<FileFilter> filters =
		FileFilter::parse("Images|*.png;*.jpg|Text|*.txt|All files|*");
	CHECK_EQ(filters.size(), (std::size_t)3);
	CHECK_EQ(filters[0].description, std::string("Images"));
	CHECK_EQ(filters[1].description, std::string("Text"));
	CHECK_EQ(filters[1].patterns.size(), (std::size_t)1);
	CHECK_EQ(filters[2].patterns[0], std::string("*"));
}

TEST(file_filter_empty_spec_is_no_filter)
{
	CHECK(FileFilter::parse("").empty());
	CHECK(FileFilter::parse("   ").empty());
}

TEST(file_filter_drops_a_description_with_no_patterns)
{
	// An odd trailing field is malformed. A filter with no patterns would match
	// nothing and hide every file, so it is dropped rather than half-honoured.
	const std::vector<FileFilter> filters = FileFilter::parse("Images|*.png|Text");
	CHECK_EQ(filters.size(), (std::size_t)1);
	CHECK_EQ(filters[0].description, std::string("Images"));
}

TEST(file_filter_drops_an_empty_pattern_group)
{
	const std::vector<FileFilter> filters = FileFilter::parse("Images||All|*");
	CHECK_EQ(filters.size(), (std::size_t)1);
	CHECK_EQ(filters[0].description, std::string("All"));
}

TEST(file_filter_trims_surrounding_space)
{
	const std::vector<FileFilter> filters = FileFilter::parse(" Images | *.png ; *.jpg ");
	CHECK_EQ(filters.size(), (std::size_t)1);
	CHECK_EQ(filters[0].description, std::string("Images"));
	CHECK_EQ(filters[0].patterns[0], std::string("*.png"));
	CHECK_EQ(filters[0].patterns[1], std::string("*.jpg"));
}

TEST(file_filter_names_itself_from_its_pattern_when_unnamed)
{
	const std::vector<FileFilter> filters = FileFilter::parse("|*.png");
	CHECK_EQ(filters.size(), (std::size_t)1);
	CHECK_EQ(filters[0].description, std::string("*.png"));
}

TEST(file_filter_matches_star_globs)
{
	const std::vector<FileFilter> filters = FileFilter::parse("Images|*.png;*.jpg");
	CHECK(filters[0].matches("photo.png"));
	CHECK(filters[0].matches("a.b.jpg"));
	CHECK(!filters[0].matches("notes.txt"));
	CHECK(!filters[0].matches("png"));
}

TEST(file_filter_match_is_case_insensitive)
{
	// The platforms disagree about whether their file systems are, so the
	// framework does not: "*.PNG" and "photo.png" match either way round.
	const std::vector<FileFilter> lower = FileFilter::parse("Images|*.png");
	CHECK(lower[0].matches("PHOTO.PNG"));
	const std::vector<FileFilter> upper = FileFilter::parse("Images|*.PNG");
	CHECK(upper[0].matches("photo.png"));
}

TEST(file_filter_matches_question_mark)
{
	const std::vector<FileFilter> filters = FileFilter::parse("Numbered|log?.txt");
	CHECK(filters[0].matches("log1.txt"));
	CHECK(!filters[0].matches("log.txt"));
	CHECK(!filters[0].matches("log12.txt"));
}

TEST(file_filter_bare_star_matches_everything)
{
	const std::vector<FileFilter> filters = FileFilter::parse("All files|*");
	CHECK(filters[0].matches("anything"));
	CHECK(filters[0].matches("a.b.c.d"));
	CHECK(filters[0].matches(""));
}

TEST(file_filter_backtracks_over_several_stars)
{
	// The glob has to retry a '*' that swallowed too little, or "*.tar.*"
	// would fail on the first extension it met.
	const std::vector<FileFilter> filters = FileFilter::parse("Tarballs|*.tar.*");
	CHECK(filters[0].matches("archive.tar.gz"));
	CHECK(filters[0].matches("a.tar.tar.bz2"));
	CHECK(!filters[0].matches("archive.zip"));
}

TEST(file_filter_with_no_patterns_matches_everything)
{
	// A default-constructed filter is "no filter", not "matches nothing" --
	// the same reading every backend gives an empty filter list.
	const FileFilter none;
	CHECK(none.matches("anything.png"));
}
