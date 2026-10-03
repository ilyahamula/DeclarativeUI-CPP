#pragma once

#include "frameworks_core/CoreTypes/GeneralTypes.hpp"

#include <array>
#include <ctime>
#include <string>

// The month-view calendar's shared rules (Calendar, CalendarWrapper): which
// weekday a date falls on, the 6x7 grid a month is drawn in for either first
// day of the week, and month stepping. ImGui draws from these; wx and Qt use
// native controls but are told the same first day, and the tests read them.

// The week's first column. Monday is ISO 8601 and most of the world; Sunday
// is the US (and Canada, Japan, ...) convention.
enum class FirstDayOfWeek
{
	Monday,
	Sunday,
};

// 0 = Sunday ... 6 = Saturday, Gregorian (Sakamoto's method).
inline int dayOfWeek(int year, int month, int day)
{
	static constexpr int kOffsets[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
	if (month < 3)
		year -= 1;
	return (year + year / 4 - year / 100 + year / 400 + kOffsets[month - 1] + day) % 7;
}

// The column a weekday (0 = Sunday) occupies when the week starts on `first`.
inline int weekdayColumn(int weekday, FirstDayOfWeek first)
{
	return first == FirstDayOfWeek::Sunday ? weekday : (weekday + 6) % 7;
}

// The month laid out the way every calendar draws it: six weeks of seven
// days, row-major, each cell the day of the month or 0 for a cell outside it.
// Six rows always, so the control never changes height from month to month.
inline std::array<int, 42> monthGrid(int year, int month, FirstDayOfWeek first)
{
	std::array<int, 42> cells {};
	const int start = weekdayColumn(dayOfWeek(year, month, 1), first);
	const int days = daysInMonth(year, month);
	for (int day = 1; day <= days; ++day)
		cells[static_cast<std::size_t>(start + day - 1)] = day;
	return cells;
}

// Column headers, English two-letter, starting at `first`. Only ImGui shows
// these: wx and Qt label their native calendars in the system's language.
inline std::array<const char*, 7> weekdayHeaders(FirstDayOfWeek first)
{
	static constexpr const char* kNames[7] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
	std::array<const char*, 7> headers {};
	for (int column = 0; column < 7; ++column)
		headers[static_cast<std::size_t>(column)] = kNames[first == FirstDayOfWeek::Sunday ? column : (column + 1) % 7];
	return headers;
}

inline const char* monthName(int month)
{
	static constexpr const char* kNames[12] = { "January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December" };
	return kNames[(month - 1 + 12) % 12];
}

// `year`/`month` moved by `delta` months (either sign).
inline void shiftMonth(int& year, int& month, int delta)
{
	const int index = year * 12 + (month - 1) + delta;
	year = index / 12;
	month = index % 12 + 1;
}

// Today in the system's local time zone -- what a Calendar opens on when it is
// given no date. localtime_r / localtime_s rather than std::chrono's zones,
// which Apple's standard library does not ship.
inline Date todayDate()
{
	const std::time_t now = std::time(nullptr);
	std::tm local {};
#ifdef _WIN32
	localtime_s(&local, &now);
#else
	localtime_r(&now, &local);
#endif
	return Date { local.tm_year + 1900, local.tm_mon + 1, local.tm_mday };
}
