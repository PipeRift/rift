#pragma once

#include <date/date.h>

#include <chrono>


namespace Rift
{
	namespace Chrono = std::chrono;

	using Milliseconds = Chrono::milliseconds;
	using Seconds = Chrono::seconds;
	using Minutes = Chrono::minutes;
	using Hours = Chrono::hours;

	using YearMonthDay = date::year_month_day;
	using Day = date::day;
	using Days = date::days;
	using Month = date::month;
	using Months = date::months;
	using Year = date::year;
	using Years = date::years;
	using WeekDay = date::weekday;
	using LocalDays = date::local_days;

	using SysDays = date::sys_days;

	constexpr const Month January = date::January;
	constexpr const Month February = date::February;
	constexpr const Month March = date::March;
	constexpr const Month April = date::April;
	constexpr const Month May = date::May;
	constexpr const Month June = date::June;
	constexpr const Month July = date::July;
	constexpr const Month August = date::August;
	constexpr const Month September = date::September;
	constexpr const Month October = date::October;
	constexpr const Month November = date::November;
	constexpr const Month December = date::December;

	constexpr const WeekDay Monday = date::Monday;
	constexpr const WeekDay Tuesday = date::Tuesday;
	constexpr const WeekDay Wednesday = date::Wednesday;
	constexpr const WeekDay Thursday = date::Thursday;
	constexpr const WeekDay Friday = date::Friday;
	constexpr const WeekDay Saturday = date::Saturday;
	constexpr const WeekDay Sunday = date::Sunday;

	constexpr const Month Jan = date::jan;
	constexpr const Month Feb = date::feb;
	constexpr const Month Mar = date::mar;
	constexpr const Month Apr = date::apr;
	// constexpr const Month May = date::may; Already defined
	constexpr const Month Jun = date::jun;
	constexpr const Month Jul = date::jul;
	constexpr const Month Aug = date::aug;
	constexpr const Month Sep = date::sep;
	constexpr const Month Oct = date::oct;
	constexpr const Month Nov = date::nov;
	constexpr const Month Dec = date::dec;

	constexpr const WeekDay Mon = date::mon;
	constexpr const WeekDay Tue = date::tue;
	constexpr const WeekDay Wed = date::wed;
	constexpr const WeekDay Thu = date::thu;
	constexpr const WeekDay Fri = date::fri;
	constexpr const WeekDay Sat = date::sat;
	constexpr const WeekDay Sun = date::sun;
}	 // namespace Rift
