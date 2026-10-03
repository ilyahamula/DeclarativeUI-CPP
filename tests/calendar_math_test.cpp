#include "test_framework.hpp"

#include "frameworks_core/CoreTypes/Calendar.hpp"

// Backend-free: the date rules the ImGui calendar draws from and every
// backend's first-day-of-week setting is checked against.

TEST(calendar_day_of_week_is_gregorian)
{
	CHECK_EQ(dayOfWeek(2026, 10, 3), 6);  // a Saturday
	CHECK_EQ(dayOfWeek(2000, 1, 1), 6);   // a Saturday
	CHECK_EQ(dayOfWeek(2024, 2, 29), 4);  // leap day, a Thursday
	CHECK_EQ(dayOfWeek(1970, 1, 1), 4);   // a Thursday
}

TEST(calendar_grid_follows_the_first_day_of_the_week)
{
	// October 2026 starts on a Thursday
	const auto monday = monthGrid(2026, 10, FirstDayOfWeek::Monday);
	CHECK_EQ(monday[3], 1);   // Mo Tu We [Th]
	CHECK_EQ(monday[2], 0);
	CHECK_EQ(monday[33], 31);
	const auto sunday = monthGrid(2026, 10, FirstDayOfWeek::Sunday);
	CHECK_EQ(sunday[4], 1);   // Su Mo Tu We [Th]
	CHECK_EQ(sunday[34], 31);

	// a month starting on the first column fills from cell 0
	const auto june = monthGrid(2026, 6, FirstDayOfWeek::Monday);   // 1 June 2026: Monday
	CHECK_EQ(june[0], 1);
	const auto february = monthGrid(2026, 2, FirstDayOfWeek::Sunday); // 1 Feb 2026: Sunday
	CHECK_EQ(february[0], 1);
	// and the 31st of a month starting late still fits the six rows
	const auto august = monthGrid(2026, 8, FirstDayOfWeek::Monday); // 1 Aug 2026: Saturday
	CHECK_EQ(august[35], 31); // column 5 + 30: the first cell of the sixth row
}

TEST(calendar_headers_and_month_steps)
{
	const auto mondayFirst = weekdayHeaders(FirstDayOfWeek::Monday);
	const auto sundayFirst = weekdayHeaders(FirstDayOfWeek::Sunday);
	CHECK_EQ(std::string(mondayFirst[0]), std::string("Mo"));
	CHECK_EQ(std::string(mondayFirst[6]), std::string("Su"));
	CHECK_EQ(std::string(sundayFirst[0]), std::string("Su"));

	int year = 2026;
	int month = 12;
	shiftMonth(year, month, 1);
	CHECK_EQ(year, 2027);
	CHECK_EQ(month, 1);
	shiftMonth(year, month, -2);
	CHECK_EQ(year, 2026);
	CHECK_EQ(month, 11);

	const Date today = todayDate();
	CHECK(today.month >= 1 && today.month <= 12);
	CHECK(today.day >= 1 && today.day <= daysInMonth(today.year, today.month));
}
