// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "Core/Platform/PlatformTime.h"

#include "Timespan.h"

class Archive;


/**
 * Enumerates the days of the week in 7-day calendars.
 */
enum class EDayOfWeek
{
	Monday = 0,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday,
	Sunday
};


/**
 * Enumerates the months of the year in 12-month calendars.
 */
enum class EMonthOfYear
{
	January = 1,
	February,
	March,
	April,
	May,
	June,
	July,
	August,
	September,
	October,
	November,
	December
};

using namespace date;
namespace Chrono = std::chrono;


/**
 * Implements a date and time.
 *
 * Values of this type represent dates and times between Midnight 00:00:00, January 1, 0001 and
 * Midnight 23:59:59.9999999, December 31, 9999 in the Gregorian calendar. Internally, the time
 * values are stored in ticks of 0.1 microseconds (= 100 nanoseconds) since January 1, 0001.
 *
 * To retrieve the current local date and time, use the FDateTime.Now() method. To retrieve the
 * current UTC time, use the FDateTime.UtcNow() method instead.
 *
 * This class also provides methods to convert dates and times from and to string representations,
 * calculate the number of days in a given month and year, check for leap years and determine the
 * time of day, day of week and month of year of a given date and time.
 *
 * The companion struct FTimespan is provided for enabling date and time based arithmetic, such as
 * calculating the difference between two dates or adding a certain amount of time to a given date.
 *
 * Ranges of dates and times can be represented by the FDateRange class.
 *
 * @see FDateRange
 * @see FTimespan
 */
struct DateTime
{
	using SysClock = Chrono::system_clock;
	using SysTime = Chrono::time_point<SysClock, decmicroseconds>;

protected:

	/** Holds the days per month in a non-leap year. */
	static const i32 DaysPerMonth[];

	/** Holds the cumulative days per month in a non-leap year. */
	static const i32 DaysToMonth[];

	static SysTime::duration utcToLocal;

	/** Holds the ticks in 100 nanoseconds resolution since January 1, 0001 A.D. */
	SysTime value;


public:

	/** Default constructor (no initialization). */
	DateTime() : value{} { }

	/**
	 * Creates and initializes a new instance with the specified number of ticks.
	 *
	 * @param Ticks The ticks representing the date and time.
	 */
	DateTime(SysTime value)
		: value{ value }
	{ }

	template<typename Precision>
	DateTime(Chrono::time_point<SysClock, Precision> value)
		: value{ Chrono::time_point_cast<decmicroseconds, SysClock, Precision>(value) }
	{ }

	/**
	 * Creates and initializes a new instance with the specified year, month, day, hour, minute, second and millisecond.
	 *
	 * @param Year The year.
	 * @param Month The month.
	 * @param Day The day.
	 * @param Hour The hour (optional).
	 * @param Minute The minute (optional).
	 * @param Second The second (optional).
	 * @param Millisecond The millisecond (optional).
	 */
	DateTime(i32 Year, i32 Month, i32 Day, i32 Hour = 0, i32 Minute = 0, i32 Second = 0, i32 Millisecond = 0);

public:

	/**
	 * Returns result of adding the given time span to this date.
	 *
	 * @return A date whose value is the sum of this date and the given time span.
	 * @see FTimespan
	 */
	DateTime operator+(const Timespan& other) const
	{
		return DateTime(value + other.GetTime());
	}

	/**
	 * Adds the given time span to this date.
	 *
	 * @return This date.
	 * @see FTimespan
	 */
	DateTime& operator+=(const Timespan& Other)
	{
		value += Other.GetTime();
		return *this;
	}

	/**
	 * Returns time span between this date and the given date.
	 *
	 * @return A time span whose value is the difference of this date and the given date.
	 * @see FTimespan
	 */
	Timespan operator-(const DateTime& other) const
	{
		return Timespan(value - other.value);
	}

	/**
	 * Returns result of subtracting the given time span from this date.
	 *
	 * @return A date whose value is the difference of this date and the given time span.
	 * @see FTimespan
	 */
	DateTime operator-(const Timespan& Other) const
	{
		return DateTime( value - Other.GetTime() );
	}

	/**
	 * Subtracts the given time span from this date.
	 *
	 * @return This date.
	 * @see FTimespan
	 */
	DateTime& operator-=(const Timespan& other)
	{
		value -= decmicroseconds{ other.GetTime() };

		return *this;
	}

	/**
	 * Compares this date with the given date for equality.
	 *
	 * @param other The date to compare with.
	 * @return true if the dates are equal, false otherwise.
	 */
	bool operator==(const DateTime& Other) const
	{
		return (value == Other.value);
	}

	/**
	 * Compares this date with the given date for inequality.
	 *
	 * @param other The date to compare with.
	 * @return true if the dates are not equal, false otherwise.
	 */
	bool operator!=(const DateTime& Other) const
	{
		return (value != Other.value);
	}

	/**
	 * Checks whether this date is greater than the given date.
	 *
	 * @param other The date to compare with.
	 * @return true if this date is greater, false otherwise.
	 */
	bool operator>(const DateTime& Other) const
	{
		return (value > Other.value);
	}

	/**
	 * Checks whether this date is greater than or equal to the date span.
	 *
	 * @param other The date to compare with.
	 * @return true if this date is greater or equal, false otherwise.
	 */
	bool operator>=(const DateTime& Other) const
	{
		return (value >= Other.value);
	}

	/**
	 * Checks whether this date is less than the given date.
	 *
	 * @param other The date to compare with.
	 * @return true if this date is less, false otherwise.
	 */
	bool operator<(const DateTime& Other) const
	{
		return (value < Other.value);
	}

	/**
	 * Checks whether this date is less than or equal to the given date.
	 *
	 * @param other The date to compare with.
	 * @return true if this date is less or equal, false otherwise.
	 */
	bool operator<=(const DateTime& Other) const
	{
		return (value <= Other.value);
	}

public:
	const SysTime& GetTime() const
	{
		return value;
	}

	i64 GetTicks() const
	{
		return value.time_since_epoch().count();
	}

	/**
	 * Gets the date part of this date.
	 *
	 * The time part is truncated and becomes 00:00:00.000.
	 *
	 * @return A FDateTime object containing the date.
	 */
	DateTime GetDate() const
	{
		return DateTime(floor<days>(value));
	}

	/**
	 * Gets the date components of this date.
	 *
	 * @param OutYear Will contain the year.
	 * @param OutMonth Will contain the number of the month (1-12).
	 * @param OutDay Will contain the number of the day (1-31).
	 */
	year_month_day GetDateComponents() const;

	/**
	 * Gets this date's day part (1 to 31).
	 *
	 * @return Day of the month.
	 * @see GetHour, GetHour12, GetMillisecond, GetMinute, GetMonth, GetSecond, GetYear
	 */
	u32 GetDay() const;

	/**
	 * Calculates this date's day of the week (Sunday - Saturday).
	 *
	 * @return The week day.
	 * @see GetDayOfYear, GetMonthOfYear, GetTimeOfDay
	 */
	EDayOfWeek GetDayOfWeek() const;

	/**
	 * Gets this date's day of the year.
	 *
	 * @return The day of year.
	 * @see GetDayOfWeek, GetMonthOfYear, GetTimeOfDay
	 */
	u32 GetDayOfYear() const;

	/**
	 * Gets this date's hour part in 24-hour clock format (0 to 23).
	 *
	 * @return The hour.
	 * @see GetDay, GetDayOfWeek, GetDayOfYear, GetHour12, GetMillisecond, GetMinute, GetMonth, GetSecond, GetYear
	 */
	u32 GetHour() const
	{
		return (floor<Chrono::hours>(value) - floor<days>(value)).count();
	}

	/**
	 * Gets this date's hour part in 12-hour clock format (1 to 12).
	 *
	 * @return The hour in AM/PM format.
	 * @see GetDay, GetHour, GetMillisecond, GetMinute, GetMonth, GetSecond, GetYear
	 */
	u32 GetHour12() const;

	/**
	 * Gets this date's millisecond part (0 to 999).
	 *
	 * @return The millisecond.
	 * @see GetDay, GetHour, GetHour12, GetMinute, GetMonth, GetSecond, GetYear
	 */
	i32 GetMillisecond() const
	{
		return (i32)(floor<Chrono::milliseconds>(value) - floor<Chrono::seconds>(value)).count();
	}

	/**
	 * Gets this date's minute part (0 to 59).
	 *
	 * @return The minute.
	 * @see GetDay, GetHour, GetHour12, GetMillisecond, GetMonth, GetSecond, GetYear
	 */
	i32 GetMinute() const
	{
		return (floor<Chrono::minutes>(value) - floor<Chrono::hours>(value)).count();
	}

	/**
	 * Gets this date's the month part (1 to 12).
	 *
	 * @return The month.
	 * @see GetDay, GetHour, GetHour12, GetMillisecond, GetMinute, GetSecond, GetYear
	 */
	u32 GetMonth() const;

	/**
	 * Gets the date's month of the year (January to December).
	 *
	 * @return Month of year.
	 * @see GetDayOfWeek, GetDayOfYear, GetTimeOfDay
	 */
	EMonthOfYear GetMonthOfYear() const
	{
		return static_cast<EMonthOfYear>(GetMonth());
	}

	/**
	 * Gets this date's second part.
	 *
	 * @return The second.
	 * @see GetDay, GetHour, GetHour12, GetMillisecond, GetMinute, GetMonth, GetYear
	 */
	i32 GetSecond() const
	{
		return (i32)(floor<Chrono::seconds>(value) - floor<Chrono::minutes>(value)).count();
	}

	/**
	 * Gets the time elapsed since midnight of this date.
	 *
	 * @param Time of day since midnight.
	 * @see GetDayOfWeek, GetDayOfYear, GetMonthOfYear
	 */
	Timespan GetTimeOfDay() const
	{
		return Timespan(value - floor<days>(value));
	}

	/**
	 * Gets this date's year part.
	 *
	 * @return The year.
	 * @see GetDay, GetHour, GetHour12, GetMillisecond, GetMinute, GetMonth, GetSecond
	 */
	i32 GetYear() const;

	/**
	 * Gets whether this date's time is in the afternoon.
	 *
	 * @param true if it is in the afternoon, false otherwise.
	 * @see IsMorning
	 */
	bool IsAfternoon() const
	{
		return (GetHour() >= 12);
	}

	/**
	 * Gets whether this date's time is in the morning.
	 *
	 * @param true if it is in the morning, false otherwise.
	 * @see IsAfternoon
	 */
	bool IsMorning() const
	{
		return (GetHour() < 12);
	}

	/**
	 * Serializes this date and time from or into the specified archive.
	 *
	 * @param Ar The archive to serialize from or into.
	 * @return true on success, false otherwise.
	 */
	bool Serialize(Archive& Ar);

	/**
	 * Returns the RFC 1123 string representation of the FDateTime.
	 *
	 * The resulting string assumes that the FDateTime is in UTC.
	 *
	 * @return String representation.
	 * @see ParseHttpDate, ToIso8601, ToString
	 */
	String ToHttpDate() const;

	/**
	 * Returns the ISO-8601 string representation of the FDateTime.
	 *
	 * The resulting string assumes that the FDateTime is in UTC.
	 *
	 * @return String representation.
	 * @see ParseIso8601, ToHttpDate, ToString
	 */
	String ToIso8601() const;

	/**
	 * Returns the string representation of this date using a default format.
	 *
	 * The returned string has the following format:
	 *		yyyy.mm.dd-hh.mm.ss
	 *
	 * @return String representation.
	 * @see Parse, ToIso8601
	 */
	String ToString() const;

	/**
	 * Returns the string representation of this date.
	 *
	 * @param Format The format of the returned string.
	 * @return String representation.
	 * @see Parse, ToIso8601
	 */
	String ToString(const TCHAR* Format) const;

	/**
	 * Returns this date as the number of seconds since the Unix Epoch (January 1st of 1970).
	 *
	 * @return Time of day.
	 * @see FromUnixTimestamp
	 */
	i64 ToUnixTimestamp() const
	{
		return floor<Chrono::seconds>( value ).time_since_epoch().count();
	}

	DateTime ToLocal() const;

public:

	/**
	 * Gets the number of days in the year and month.
	 *
	 * @param Year The year.
	 * @param Month The month.
	 * @return The number of days
	 * @see DaysInYear
	 */
	static i32 DaysInMonth(i32 Year, i32 Month);

	/**
	 * Gets the number of days in the given year.
	 *
	 * @param Year The year.
	 * @return The number of days.
	 * @see DaysInMonth
	 */
	static i32 DaysInYear(i32 Year);


	/**
	 * Returns the date from Unix time (seconds from midnight 1970-01-01)
	 *
	 * @param UnixTime Unix time (seconds from midnight 1970-01-01)
	 * @return Gregorian date and time.
	 * @see ToUnixTimestamp
	 */
	static DateTime FromUnixTimestamp(i64 UnixTime)
	{
		return DateTime(1970, 1, 1) + Timespan(Chrono::seconds{ UnixTime });
	}

	/**
	 * Checks whether the given year is a leap year.
	 *
	 * A leap year is a year containing one additional day in order to keep the calendar synchronized
	 * with the astronomical year. All years divisible by 4, but not divisible by 100 - except if they
	 * are also divisible by 400 - are leap years.
	 *
	 * @param Year The year to check.
	 * @return true if the year is a leap year, false otherwise.
	 */
	static bool IsLeapYear(i32 Year);

	/**
	 * Returns the maximum date value.
	 *
	 * The maximum date value is December 31, 9999, 23:59:59.9999999.
	 *
	 * @see MinValue
	 */
	static DateTime MaxValue()
	{
		return DateTime(SysTime::max());
	}

	/**
	 * Returns the minimum date value.
	 *
	 * The minimum date value is January 1, 0001, 00:00:00.0.
	 *
	 * @see MaxValue
	 */
	static DateTime MinValue()
	{
		return DateTime(SysTime::min());
	}

	/**
	 * Gets the local date and time on this computer.
	 *
	 * This method takes into account the local computer's time zone and daylight saving
	 * settings. For time zone independent time comparisons, and when comparing times
	 * between different computers, please use UtcNow() instead.
	 *
	 * @return Current date and time.
	 * @see Today, UtcNow
	 */
	static DateTime Now();

	/**
	 * Gets the current UTC date and time.
	 *
	 * @return Current date and time.
	 * @see Today, Now
	 */
	static DateTime UtcNow();

	/**
	 * Converts a string to a date and time.
	 *
	 * Currently, the string must be in the format written by either FDateTime.ToString() or
	 * FTimeStamp.TimestampToFString(). Other formats are not supported at this time.
	 *
	 * @param DateTimeString The string to convert.
	 * @param OutDateTime Will contain the parsed date and time.
	 * @return true if the string was converted successfully, false otherwise.
	 * @see ParseHttpDate, ParseIso8601, ToString
	 */
	static bool Parse(const String& DateTimeString, DateTime& OutDateTime);

	/**
	 * Parses a date string in HTTP-date format (rfc1123-date | rfc850-date | asctime-date)
	 * https://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
	 *
	 * HTTP-date    = rfc1123-date | rfc850-date | asctime-date
	 * rfc1123-date = wkday "," SP date1 SP time SP "GMT"
	 * rfc850-date  = weekday "," SP date2 SP time SP "GMT"
	 * asctime-date = wkday SP date3 SP time SP 4DIGIT
	 * date1        = 2DIGIT SP month SP 4DIGIT ; day month year (e.g., 02 Jun 1982)
	 * date2        = 2DIGIT "-" month "-" 2DIGIT ; day-month-year (e.g., 02-Jun-82)
	 * date3        = month SP (2DIGIT | (SP 1DIGIT)) ; month day (e.g., Jun  2)
	 * time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT ; 00:00:00 - 23:59:59
	 * wkday        = "Mon" | "Tue" | "Wed" | "Thu" | "Fri" | "Sat" | "Sun"
	 * weekday      = "Monday" | "Tuesday" | "Wednesday" | "Thursday" | "Friday" | "Saturday" | "Sunday"
	 * month        = "Jan" | "Feb" | "Mar" | "Apr" | "May" | "Jun" | "Jul" | "Aug" | "Sep" | "Oct" | "Nov" | "Dec"
	 *
	 * @param HttpDate The string to be parsed
	 * @param OutDateTime FDateTime object (assumes UTC) corresponding to the input string.
	 * @return true if the string was converted successfully, false otherwise.
	 * @see Parse, ToHttpDate, ParseIso8601
	 */
	static bool ParseHttpDate(const String& HttpDate, DateTime& OutDateTime);

	/**
	 * Parses a date string in ISO-8601 format.
	 *
	 * @param DateTimeString The string to be parsed
	 * @param OutDateTime FDateTime object (in UTC) corresponding to the input string (which may have been in any timezone).
	 * @return true if the string was converted successfully, false otherwise.
	 * @see Parse, ParseHttpDate, ToIso8601
	 */
	static bool ParseIso8601(const TCHAR* DateTimeString, DateTime& OutDateTime);

	/**
	 * Gets the local date on this computer.
	 *
	 * The time component is set to 00:00:00
	 *
	 * @return Current date.
	 * @see Now, UtcNow
	 */
	static DateTime Today()
	{
		return Now().GetDate();
	}

	/**
	 * Validates the given components of a date and time value.
	 *
	 * The allow ranges for the components are:
	 *		Year: 1 - 9999
	 *		Month: 1 - 12
	 *		Day: 1 - DaysInMonth(Month)
	 *		Hour: 0 - 23
	 *		Minute: 0 - 59
	 *		Second: 0 - 59
	 *		Millisecond: 0 - 999
	 *
	 * @return true if the components are valid, false otherwise.
	 */
	static bool Validate(i32 Year, i32 Month, i32 Day, i32 Hour, i32 Minute, i32 Second, i32 Millisecond);

	static void InitializeTime();

	template<typename Clock, typename Precision>
	static DateTime CastClock(Chrono::time_point<Clock, Precision> time)
	{
		return { clock_cast<decmicroseconds, Precision, SysClock, Clock>(time) };
	}

protected:

	/**
	 * Cast to a different type of clock.
	 * This operation may contains small precision errors.
	 */
	template <
		typename DstDuration,
		typename SrcDuration,
		typename DstClock,
		typename SrcClock,
		typename DstTime = Chrono::time_point<DstClock, DstDuration>,
		typename SrcTime = Chrono::time_point<SrcClock, SrcDuration>
	>
	static DstTime clock_cast(const SrcTime tp, const SrcDuration tolerance = decmicroseconds{ 1 }, const i32 limit = 10)
	{
		assert(limit > 0);
		auto itercnt = 0;
		auto src_now = SrcTime{};
		auto dst_now = DstTime{};

		SrcDuration epsilon = SrcDuration::max();
		do
		{
			const auto src_before = SrcClock::now();
			const auto dst_between = DstClock::now();
			const auto src_after = SrcClock::now();
			const auto src_diff = src_after - src_before;
			const auto delta = Chrono::abs(src_diff);
			if (delta < epsilon)
			{
				src_now = src_before + src_diff / 2;
				dst_now = dst_between;
				epsilon = delta;
			}
			if (++itercnt >= limit)
				break;
		} while (epsilon > tolerance);

		return dst_now + (tp - src_now);
	}

private:
	friend struct Z_Construct_UScriptStruct_FDateTime_Statics;
};
