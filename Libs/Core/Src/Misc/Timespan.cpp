// Copyright 2015-2020 Piperift - All rights reserved

#include "Misc/Timespan.h"


namespace Rift
{
	/* FTimespan interface
	 *****************************************************************************/

	String Timespan::ToString() const
	{
		if (GetDays() == 0)
		{
			return ToString(TX("%h:%m:%s.%f"));
		}

		return ToString(TX("%d.%h:%m:%s.%f"));
	}


	String Timespan::ToString(const TCHAR* format) const
	{
		String result;

		result += (*this < Timespan::Zero()) ? TX('-') : TX('+');

		while (*format != TX('\0'))
		{
			if ((*format == TX('%')) && (*++format != TX('\0')))
			{
				switch (*format)
				{
					case TX('d'):
						CString::FormatTo(result, TX("{}"), Math::Abs(GetDays()));
						break;
					case TX('D'):
						CString::FormatTo(result, TX("{:08i}"), Math::Abs(GetDays()));
						break;
					case TX('h'):
						CString::FormatTo(result, TX("{:02i}"), Math::Abs(GetHours()));
						break;
					case TX('m'):
						CString::FormatTo(result, TX("{:02i}"), Math::Abs(GetMinutes()));
						break;
					case TX('s'):
						CString::FormatTo(result, TX("{:02i}"), Math::Abs(GetSeconds()));
						break;
					case TX('f'):
						CString::FormatTo(result, TX("{:03i}"), Math::Abs(GetFractionMilli()));
						break;
					case TX('u'):
						CString::FormatTo(result, TX("{:06i}"), Math::Abs(GetFractionMicro()));
						break;
					case TX('t'):
						CString::FormatTo(result, TX("{:07i}"), Math::Abs(GetFractionTicks()));
						break;
					case TX('n'):
						CString::FormatTo(result, TX("{:09i}"), Math::Abs(GetFractionNano()));
						break;
					default:
						result += *format;
				}
			}
			else
			{
				result += *format;
			}

			++format;
		}

		return result;
	}


	/* FTimespan static interface
	 *****************************************************************************/

	bool Timespan::Parse(const String& TimespanString, Timespan& OutTimespan)
	{
		// @todo gmp: implement stricter FTimespan parsing; this implementation is too forgiving

		// get string tokens
		const bool HasFractional = CString::Contains(TimespanString, TX('.')) ||
								   CString::Contains(TimespanString, TX(','));
		String TokenString = TimespanString;
		CString::Replace(TokenString, TX('.'), TX(':'));
		CString::Replace(TokenString, TX(','), TX(':'));

		const bool Negative = TokenString[0] == TX('-');
		CString::Replace(TokenString, TX('-'), TX(':'));
		CString::Replace(TokenString, TX('+'), TX(':'));

		TArray<String> Tokens;
		CString::Split(TokenString, Tokens, TX(':'));

		if (!HasFractional)
		{
			Tokens.AddDefaulted();
		}

		// poor man's token verification
		for (const String& token : Tokens)
		{
			if (!token.empty() && !CString::IsNumeric(token))
			{
				return false;
			}
		}

		// add missing tokens
		if (Tokens.Size() < 5)
		{
			Tokens.InsertDefaulted(0, 5 - Tokens.Size());
		}
		else if (Tokens.Size() > 5)
		{
			return false;
		}

		// pad fractional token with zeros
		if (HasFractional)
		{
			const i32 FractionalLen = (i32)(Tokens[4].size());

			if (FractionalLen > 9)
			{
				Tokens[4] = CString::FrontSubstr(Tokens[4], 9);
			}
			else if (FractionalLen < 9)
			{
				Tokens[4] += CString::FrontSubstr({TX("000000000")}, 9 - FractionalLen);
			}
		}

		const i32 days = *CString::ToI32(Tokens[0]);
		const i32 hours = *CString::ToI32(Tokens[1]);
		const i32 minutes = *CString::ToI32(Tokens[2]);
		const i32 seconds = *CString::ToI32(Tokens[3]);
		const i32 fractionNano = *CString::ToI32(Tokens[4]);

		// Max days
		if ((days > Chrono::floor<Days>(decmicroseconds::max()).count() - 1))
		{
			return false;
		}

		if ((hours > 23) || (minutes > 59) || (seconds > 59) || (fractionNano > 999999999))
		{
			return false;
		}

		OutTimespan.Assign(days, hours, minutes, seconds, fractionNano);

		if (Negative)
		{
			OutTimespan.duration *= -1;
		}

		return true;
	}


	/* FTimespan implementation
	 *****************************************************************************/

	void Timespan::Assign(i32 days, i32 hours, i32 minutes, i32 seconds, i32 fractionNano)
	{
		duration = Chrono::floor<decmicroseconds>(
			Days{days} + Chrono::hours{hours} + Chrono::minutes{minutes} +
			Chrono::seconds{seconds} + Chrono::nanoseconds{fractionNano});
	}

	Timespan Timespan::FromHours(i32 hours)
	{
		return Timespan{hours, 0, 0};
	}

	Timespan Timespan::FromMinutes(i32 minutes)
	{
		return Timespan{0, minutes, 0};
	}

	Timespan Timespan::FromSeconds(i32 seconds)
	{
		return Timespan{0, 0, seconds};
	}
}	 // namespace Rift
