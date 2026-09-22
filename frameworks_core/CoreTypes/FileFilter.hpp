#pragma once

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

// A file dialog's type filter, parsed once from caller text such as
//
//     "Images (*.png;*.jpg)|*.png;*.jpg|All files|*"
//
// and handed to every backend as data. This is the same shape Shortcut has and
// for the same reason: the three backends spell a wildcard string differently
// (wx wants "desc|*.png;*.jpg|desc|*", Qt wants "desc (*.png *.jpg);;desc (*)")
// and only the backend knows its own spelling, so re-parsing per backend would
// mean three parsers that have to agree. Parse once, map the FIELDS.
//
// The spec grammar is wx's, because that is the spelling callers already know:
// alternating description / pattern-list pairs separated by '|', with the
// patterns inside a group separated by ';'. A pattern is a glob over '*' and
// '?' -- all three native dialogs agree on those and on nothing else.
struct FileFilter
{
	std::string description;           // "Images"
	std::vector<std::string> patterns; // { "*.png", "*.jpg" }

	bool operator==(const FileFilter&) const = default;

	// Parses the spec above. Never throws and never half-honours a malformed
	// spec -- the text arrives from a literal that nothing validated, so a
	// trailing description with no pattern list is DROPPED rather than given an
	// empty pattern set that would match nothing and hide every file.
	// An empty spec yields an empty vector, which every backend reads as
	// "no filter".
	static std::vector<FileFilter> parse(std::string_view spec);

	// Does `filename` (a leaf name, not a path) match any of this filter's
	// patterns? Case-insensitive, because the platforms disagree about whether
	// their file systems are. Used by the ImGui browser, which has to do its
	// own filtering; wx and Qt hand the patterns to the native dialog instead.
	bool matches(std::string_view filename) const;
};

namespace file_filter_detail
{

inline std::string_view trimmed(std::string_view text)
{
	while (!text.empty() && std::isspace((unsigned char)text.front()))
		text.remove_prefix(1);
	while (!text.empty() && std::isspace((unsigned char)text.back()))
		text.remove_suffix(1);
	return text;
}

// Split on `sep`, trimming each field and dropping the empty ones. Used for the
// pattern list only: the '|' split has to keep position, since description and
// patterns alternate.
inline std::vector<std::string> splitPatterns(std::string_view list, char sep)
{
	std::vector<std::string> out;
	std::string_view rest = list;
	while (true)
	{
		const std::size_t at = rest.find(sep);
		const std::string_view token = trimmed(at == std::string_view::npos ? rest : rest.substr(0, at));
		if (!token.empty())
			out.emplace_back(token);
		if (at == std::string_view::npos)
			break;
		rest = rest.substr(at + 1);
	}
	return out;
}

// Classic '*' / '?' glob, iterative with a backtrack point so a pattern like
// "*.tar.*" cannot blow the stack on a long name. Both sides arrive already
// lowered by the caller.
inline bool globMatch(std::string_view pattern, std::string_view text)
{
	std::size_t p = 0;
	std::size_t t = 0;
	std::size_t starP = std::string_view::npos;
	std::size_t starT = 0;

	while (t < text.size())
	{
		if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t]))
		{
			++p;
			++t;
		}
		else if (p < pattern.size() && pattern[p] == '*')
		{
			starP = p++;
			starT = t;
		}
		else if (starP != std::string_view::npos)
		{
			// Re-try the last '*' having it swallow one more character.
			p = starP + 1;
			t = ++starT;
		}
		else
		{
			return false;
		}
	}

	while (p < pattern.size() && pattern[p] == '*')
		++p;
	return p == pattern.size();
}

inline std::string lowered(std::string_view text)
{
	std::string out;
	out.reserve(text.size());
	for (const char c : text)
		out.push_back((char)std::tolower((unsigned char)c));
	return out;
}

} // namespace file_filter_detail

inline std::vector<FileFilter> FileFilter::parse(std::string_view spec)
{
	std::vector<FileFilter> filters;

	// Fields alternate description, patterns, description, patterns...
	std::vector<std::string_view> fields;
	std::string_view rest = spec;
	while (true)
	{
		const std::size_t bar = rest.find('|');
		fields.push_back(bar == std::string_view::npos ? rest : rest.substr(0, bar));
		if (bar == std::string_view::npos)
			break;
		rest = rest.substr(bar + 1);
	}

	// A lone empty field is an empty spec, not a filter with no name.
	if (fields.size() == 1 && file_filter_detail::trimmed(fields.front()).empty())
		return filters;

	// Deliberately `+ 1 < size`, so an odd trailing description is dropped: a
	// filter with no patterns would match nothing and hide every file, which is
	// worse than not offering it at all.
	for (std::size_t i = 0; i + 1 < fields.size(); i += 2)
	{
		FileFilter filter;
		filter.description = std::string(file_filter_detail::trimmed(fields[i]));
		filter.patterns = file_filter_detail::splitPatterns(fields[i + 1], ';');
		if (filter.patterns.empty())
			continue; // "Images||All|*" -- the empty group is not a filter
		if (filter.description.empty())
			filter.description = filter.patterns.front();
		filters.push_back(std::move(filter));
	}
	return filters;
}

inline bool FileFilter::matches(std::string_view filename) const
{
	if (patterns.empty())
		return true;
	const std::string name = file_filter_detail::lowered(filename);
	for (const std::string& pattern : patterns)
	{
		if (file_filter_detail::globMatch(file_filter_detail::lowered(pattern), name))
			return true;
	}
	return false;
}
