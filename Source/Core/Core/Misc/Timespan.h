// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"

#include <ratio>
#include <chrono>
#include <date/date.h>

#include "Core/Math/Math.h"
#include "Core/Strings/String.h"


// decimals of microseconds
using decmicroseconds = std::chrono::duration<i64, std::ratio<1, 10'000'000>>;


/**
 * Implements a time span.
 *
 * A time span is the difference between two dates and times. For example, the time span between
 * 12:00:00 January 1, 2000 and 18:00:00 January 2, 2000 is 30.0 hours. Time spans are measured in
 * positive or negative ticks depending on whether the difference is measured forward or backward.
 * Each tick has a resolution of 0.1 microseconds (= 100 nanoseconds).
 *
 * In conjunction with the companion class DateTime, time spans can be used to perform date and time
 * based arithmetic, such as calculating the difference between two dates or adding a certain amount
 * of time to a given date.
 *
 * When initializing time span values from single components, consider using the FromHours, FromMinutes,
 * FromSeconds, Zero, MinValue and related methods instead of calling the overloaded constructors as
 * they will make your code easier to read and understand.
 *
 * @see DateTime
 */
struct Timespan
{
	using SysClock = std::chrono::system_clock;
	using SysDuration = std::chrono::duration<SysClock, decmicroseconds>;


public:

	/** Default constructor (no initialization). */
	Timespan() { }

	/**
	 * Create and initialize a new time interval with the specified number of ticks.
	 *
	 * For better readability, consider using MinValue, MaxValue and Zero.
	 *
	 * @param duration The number of decimals of microseconds.
	 * @see MaxValue, MinValue, Zero
	 */
	Timespan(decmicroseconds duration) : duration(duration) {}

	/**
	 * Create and initialize a new time interval with the specified number of hours, minutes and seconds.
	 *
	 * For better readability, consider using FromHours, FromMinutes and FromSeconds.
	 *
	 * @param Hours The hours component.
	 * @param Minutes The minutes component.
	 * @param Seconds The seconds component.
	 * @see FromHours, FromMinutes, FromSeconds
	 */
	Timespan(i32 Hours, i32 Minutes, i32 Seconds)
	{
		Assign(0, Hours, Minutes, Seconds, 0);
	}

	/**
	 * Create and initialize a new time interval with the specified number of days, hours, minutes and seconds.
	 *
	 * For better readability, consider using FromDays, FromHours, FromMinutes and FromSeconds.
	 *
	 * @param Days The days component.
	 * @param Hours The hours component.
	 * @param Minutes The minutes component.
	 * @param Seconds The seconds component.
	 * @see FromDays, FromHours, FromMinutes, FromSeconds
	 */
	Timespan(i32 Days, i32 Hours, i32 Minutes, i32 Seconds)
	{
		Assign(Days, Hours, Minutes, Seconds, 0);
	}

	/**
	 * Create and initialize a new time interval with the specified number of days, hours, minutes and seconds.
	 *
	 * @param Days The days component.
	 * @param Hours The hours component.
	 * @param Minutes The minutes component.
	 * @param Seconds The seconds component.
	 * @param FractionNano The fractional seconds (in nanosecond resolution).
	 */
	Timespan(i32 Days, i32 Hours, i32 Minutes, i32 Seconds, i32 FractionNano)
	{
		Assign(Days, Hours, Minutes, Seconds, FractionNano);
	}

public:

	/**
	 * Return the result of adding the given time span to this time span.
	 *
	 * @return A time span whose value is the sum of this time span and the given time span.
	 */
	Timespan operator+(const Timespan& Other) const
	{
		return { duration + Other.duration };
	}

	/**
	 * Adds the given time span to this time span.
	 *
	 * @return This time span.
	 */
	Timespan& operator+=(const Timespan& Other)
	{
		duration += Other.duration;
		return *this;
	}

	/**
	 * Return the inverse of this time span.
	 *
	 * The value of this time span must be greater than FTimespan::MinValue(), or else an overflow will occur.
	 *
	 * @return Inverse of this time span.
	 */
	Timespan operator-() const
	{
		return { -duration };
	}

	/**
	 * Return the result of subtracting the given time span from this time span.
	 *
	 * @param Other The time span to compare with.
	 * @return A time span whose value is the difference of this time span and the given time span.
	 */
	Timespan operator-(const Timespan& Other) const
	{
		return { duration - Other.duration };
	}

	/**
	 * Subtract the given time span from this time span.
	 *
	 * @param Other The time span to subtract.
	 * @return This time span.
	 */
	Timespan& operator-=(const Timespan& Other)
	{
		duration -= Other.duration;
		return *this;
	}

	/**
	 * Return the result of multiplying the this time span with the given scalar.
	 *
	 * @param Scalar The scalar to multiply with.
	 * @return A time span whose value is the product of this time span and the given scalar.
	 */
	Timespan operator*(double scalar) const
	{
		return Timespan( decmicroseconds{ (i64)(duration.count() * scalar) });
	}

	/**
	 * Multiply this time span with the given scalar.
	 *
	 * @param Scalar The scalar to multiply with.
	 * @return This time span.
	 */
	Timespan& operator*=(double scalar)
	{
		duration = decmicroseconds{ (i64)(duration.count() * scalar) };
		return *this;
	}

	/**
	 * Return the result of dividing the this time span by the given scalar.
	 *
	 * @param Scalar The scalar to divide by.
	 * @return A time span whose value is the quotient of this time span and the given scalar.
	 */
	Timespan operator/(double scalar) const
	{
		return Timespan(decmicroseconds{ (i64)(duration.count() / scalar) });
	}

	/**
	 * Divide this time span by the given scalar.
	 *
	 * @param Scalar The scalar to divide by.
	 * @return This time span.
	 */
	Timespan& operator/=(double scalar)
	{
		duration = decmicroseconds{ (i64)(duration.count() / scalar) };
		return *this;
	}

	/**
	 * Return the result of calculating the modulus of this time span with another time span.
	 *
	 * @param Other The time span to divide by.
	 * @return A time span representing the remainder of the modulus operation.
	 */
	Timespan operator%(const Timespan& Other) const
	{
		return Timespan(duration % Other.duration);
	}

	/**
	 * Calculate this time span modulo another.
	 *
	 * @param Other The time span to divide by.
	 * @return This time span.
	 */
	Timespan& operator%=(const Timespan& Other)
	{
		duration %= Other.duration;
		return *this;
	}

	/**
	 * Compare this time span with the given time span for equality.
	 *
	 * @param Other The time span to compare with.
	 * @return true if the time spans are equal, false otherwise.
	 */
	bool operator==(const Timespan& Other) const
	{
		return (duration == Other.duration);
	}

	/**
	 * Compare this time span with the given time span for inequality.
	 *
	 * @param Other The time span to compare with.
	 * @return true if the time spans are not equal, false otherwise.
	 */
	bool operator!=(const Timespan& Other) const
	{
		return (duration != Other.duration);
	}

	/**
	 * Check whether this time span is greater than the given time span.
	 *
	 * @param Other The time span to compare with.
	 * @return true if this time span is greater, false otherwise.
	 */
	bool operator>(const Timespan& Other) const
	{
		return (duration > Other.duration);
	}

	/**
	 * Check whether this time span is greater than or equal to the given time span.
	 *
	 * @param Other The time span to compare with.
	 * @return true if this time span is greater or equal, false otherwise.
	 */
	bool operator>=(const Timespan& other) const
	{
		return (duration >= other.duration);
	}

	/**
	 * Check whether this time span is less than the given time span.
	 *
	 * @param Other The time span to compare with.
	 * @return true if this time span is less, false otherwise.
	 */
	bool operator<(const Timespan& other) const
	{
		return (duration < other.duration);
	}

	/**
	 * Check whether this time span is less than or equal to the given time span.
	 *
	 * @param Other The time span to compare with.
	 * @return true if this time span is less or equal, false otherwise.
	 */
	bool operator<=(const Timespan& other) const
	{
		return (duration <= other.duration);
	}

public:

	/**
	 * Get the days component of this time span.
	 *
	 * @return Days component.
	 */
	i32 GetDays() const
	{
		return floor<date::days>(duration).count();
	}

	/**
	 * Get a time span with the absolute value of this time span.
	 *
	 * This method may overflow the timespan if its value is equal to MinValue.
	 *
	 * @return Duration of this time span.
	 * @see MinValue
	 */
	Timespan GetDuration()
	{
		return Timespan(duration.count() >= 0 ? duration : -duration);
	}

	/**
	 * Gets the fractional seconds (in microsecond resolution).
	 *
	 * @return Number of microseconds in fractional part.
	 * @see GetTotalMicroseconds
	 */
	i64 GetFractionMicro() const
	{
		return (floor<std::chrono::microseconds>(duration) - floor<std::chrono::seconds>(duration)).count();
	}

	/**
	 * Gets the fractional seconds (in millisecond resolution).
	 *
	 * @return Number of milliseconds in fractional part.
	 * @see GetTotalMilliseconds
	 */
	i64 GetFractionMilli() const
	{
		return (floor<std::chrono::milliseconds>(duration) - floor<std::chrono::seconds>(duration)).count();
	}

	/**
	 * Gets the fractional seconds (in nanosecond resolution).
	 *
	 * @return Number of nanoseconds in fractional part.
	 */
	i64 GetFractionNano() const
	{
		return (floor<std::chrono::nanoseconds>(duration) - floor<std::chrono::seconds>(duration)).count();
	}

	/**
	 * Gets the fractional ticks (in 100 nanosecond resolution).
	 *
	 * @return Number of ticks in fractional part.
	 */
	i64 GetFractionTicks() const
	{
		return (duration - floor<std::chrono::seconds>(duration)).count();
	}

	/**
	 * Gets the hours component of this time span.
	 *
	 * @return Hours component.
	 * @see GetTotalHours
	 */
	i32 GetHours() const
	{
		return (i32)(floor<std::chrono::hours>(duration) - floor<date::days>(duration)).count();
	}

	/**
	 * Get the minutes component of this time span.
	 *
	 * @return Minutes component.
	 * @see GetTotalMinutes
	 */
	i32 GetMinutes() const
	{
		return (i32)(floor<std::chrono::minutes>(duration) - floor<std::chrono::hours>(duration)).count();
	}

	/**
	 * Get the seconds component of this time span.
	 *
	 * @return Seconds component.
	 * @see GetTotalSeconds
	 */
	i32 GetSeconds() const
	{
		return (i32)(floor<std::chrono::seconds>(duration) - floor<std::chrono::minutes>(duration)).count();
	}

	/**
	 * Get the number of ticks represented by this time span.
	 *
	 * @return Number of ticks.
	 */
	const decmicroseconds& GetTime() const
	{
		return duration;
	}


	/**
	 * Get the total number of days represented by this time span.
	 *
	 * @return Number of days.
	 * @see GetDays
	 */
	template<typename PrecisionType = float>
	PrecisionType GetTotalDays() const
	{
		return std::chrono::duration<PrecisionType, date::days::period>(duration).count();
	}

	/**
	 * Get the total number of hours represented by this time span.
	 *
	 * @return Number of hours.
	 * @see GetHours
	 */
	template<typename PrecisionType = float>
	PrecisionType GetTotalHours() const
	{
		return std::chrono::duration<PrecisionType, std::chrono::hours::period>(duration).count();
	}

	/**
	 * Get the total number of microseconds represented by this time span.
	 *
	 * @return Number of microseconds.
	 * @see GetFractionMicro
	 */
	template<typename PrecisionType = float>
	PrecisionType GetTotalMicroseconds() const
	{
		return std::chrono::duration<PrecisionType, std::chrono::microseconds::period>(duration).count();
	}

	/**
	 * Get the total number of milliseconds represented by this time span.
	 *
	 * @return Number of milliseconds.
	 * @see GetFractionMilli
	 */
	template<typename PrecisionType = float>
	PrecisionType GetTotalMilliseconds() const
	{
		return std::chrono::duration<PrecisionType, std::chrono::milliseconds::period>(duration).count();
	}

	/**
	 * Get the total number of minutes represented by this time span.
	 *
	 * @return Number of minutes.
	 * @see GetMinutes
	 */
	template<typename PrecisionType = float>
	PrecisionType GetTotalMinutes() const
	{
		return std::chrono::duration<PrecisionType, std::chrono::minutes::period>(duration).count();
	}

	/**
	 * Get the total number of seconds represented by this time span.
	 *
	 * @return Number of seconds.
	 * @see GetSeconds
	 */
	template<typename PrecisionType = float>
	PrecisionType GetTotalSeconds() const
	{
		return std::chrono::duration<PrecisionType, std::chrono::seconds::period>(duration).count();
	}

	/**
	 * Check whether this time span is zero.
	 *
	 * @return true if the time span is zero, false otherwise.
	 * @see Zero
	 */
	bool IsZero() const
	{
		return duration == decmicroseconds::zero();
	}

	/**
	 * Return the string representation of this time span using a default format.
	 *
	 * The returned string has the following format:
	 *		p[d.]hh:mm:ss.fff
	 *
	 * Note that 'p' is the plus or minus sign, and the date component is
	 * omitted for time spans that are shorter than one day.
	 *
	 * Examples:
	 *      -42.15:11:36.457 (45 days, 15 hours, 11 minutes, 36.457 seconds in the past)
	 *      +42.15:11:36.457 (45 days, 15 hours, 11 minutes, 36.457 seconds in the future)
	 *      +15:11:36.457 (15 hours, 11 minutes, 36.457 seconds in the future)
	 *      +00:11:36.457 (11 minutes, 36.457 seconds in the future)
	 *      +00:00:36.457 (36.457 seconds in the future)
	 *
	 * @return String representation.
	 * @see Parse
	 */
	String ToString() const;

	/**
	 * Convert this time span to its string representation.
	 *
	 * The following formatting codes are available:
	 *		%d - prints the days component
	 *		%D - prints the zero-padded days component (00000000..10675199)
	 *		%h - prints the zero-padded hours component (00..23)
	 *		%m - prints the zero-padded minutes component (00..59)
	 *		%s - prints the zero-padded seconds component (00..59)
	 *		%f - prints the zero-padded fractional seconds (000..999)
	 *		%u - prints the zero-padded fractional seconds (000000..999999)
	 *		%n - prints the zero-padded fractional seconds (000000000..999999999)
	 *
	 * Depending on whether the time span is positive or negative, a plus or minus
	 * sign character will always be added in front of the generated string.
	 *
	 * @param Format The format of the returned string.
	 * @return String representation.
	 * @see Parse
	 */
	String ToString(const TCHAR* Format) const;

public:

	/**
	 * Return the maximum time span value.
	 *
	 * The maximum time span value is slightly more than 10,675,199 days.
	 *
	 * @return Maximum time span.
	 * @see MinValue,Zero
	 */
	static Timespan MaxValue()
	{
		return Timespan(decmicroseconds::max());
	}

	/**
	 * Return the minimum time span value.
	 *
	 * The minimum time span value is slightly less than -10,675,199 days.
	 *
	 * @return Minimum time span.
	 * @see MaxValue, ZeroValue
	 */
	static Timespan MinValue()
	{
		return Timespan(decmicroseconds::min());
	}

	/**
	 * Convert a string to a time span.
	 *
	 * The string must be in one of the following formats:
	 *    p[d.]hh::mm::ss.fff
	 *    p[d.]hh::mm::ss.uuuuuu
	 *    p[d.]hh::mm::ss.nnnnnnnnn
	 *
	 * Note that 'p' is the plus or minus sign, and the date component may be
	 * omitted for time spans that are shorter than one day.
	 *
	 * @param TimespanString The string to convert.
	 * @param OutTimespan Will contain the parsed time span.
	 * @return true if the string was converted successfully, false otherwise.
	 * @see ToString
	 */
	static bool Parse(const String& TimespanString, Timespan& OutTimespan);

	/**
	 * Return the zero time span value.
	 *
	 * The zero time span value can be used in comparison operations with other time spans.
	 *
	 * @return Zero time span.
	 * @see IsZero, MaxValue, MinValue
	 */
	static Timespan Zero()
	{
		return Timespan(decmicroseconds::zero());
	}

public:

	friend class UObject;

protected:

	/**
	 * Assign the specified components to this time span.
	 *
	 * @param Days The days component.
	 * @param Hours The hours component.
	 * @param Minutes The minutes component.
	 * @param Seconds The seconds component.
	 * @param FractionNano The fractional seconds (in nanosecond resolution).
	 */
	void Assign(i32 days, i32 hours, i32 minutes, i32 seconds, i32 fractionNano);

private:
	friend struct Z_Construct_UScriptStruct_FTimespan_Statics;

private:

	/** The time span value in 100 nanoseconds resolution. */
	decmicroseconds duration;
public:
	static Timespan FromHours(i32 hours);
	static Timespan FromMinutes(i32 TzOffsetMinutes);
	static Timespan FromSeconds(i32 seconds);
};


/**
 * Pre-multiply a time span with the given scalar.
 *
 * @param Scalar The scalar to pre-multiply with.
 * @param Timespan The time span to multiply.
 */
inline Timespan operator*(float Scalar, const Timespan& Timespan)
{
	return Timespan.operator*(Scalar);
}
