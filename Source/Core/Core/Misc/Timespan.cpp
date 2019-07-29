// Copyright 2015-2019 Piperift - All rights reserved

#include "Timespan.h"

using namespace EA::StdC;


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


String Timespan::ToString(const TCHAR* Format) const
{
	String Result;

	Result += (*this < Timespan::Zero()) ? TX('-') : TX('+');

	while (*Format != TX('\0'))
	{
		if ((*Format == TX('%')) && (*++Format != TX('\0')))
		{
			switch (*Format)
			{
			case TX('d'): Result.append_sprintf(TX("%i"),   Math::Abs(GetDays())); break;
			case TX('D'): Result.append_sprintf(TX("%08i"), Math::Abs(GetDays())); break;
			case TX('h'): Result.append_sprintf(TX("%02i"), Math::Abs(GetHours())); break;
			case TX('m'): Result.append_sprintf(TX("%02i"), Math::Abs(GetMinutes())); break;
			case TX('s'): Result.append_sprintf(TX("%02i"), Math::Abs(GetSeconds())); break;
			case TX('f'): Result.append_sprintf(TX("%03i"), Math::Abs(GetFractionMilli())); break;
			case TX('u'): Result.append_sprintf(TX("%06i"), Math::Abs(GetFractionMicro())); break;
			case TX('t'): Result.append_sprintf(TX("%07i"), Math::Abs(GetFractionTicks())); break;
			case TX('n'): Result.append_sprintf(TX("%09i"), Math::Abs(GetFractionNano())); break;
			default:
				Result += *Format;
			}
		}
		else
		{
			Result += *Format;
		}

		++Format;
	}

	return Result;
}


/* FTimespan static interface
 *****************************************************************************/

bool Timespan::Parse(const String& TimespanString, Timespan& OutTimespan)
{
	// @todo gmp: implement stricter FTimespan parsing; this implementation is too forgiving

	// get string tokens
	const bool HasFractional = CString::Contains(TimespanString, TX('.')) || CString::Contains(TimespanString, TX(','));
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
			Tokens[4] += CString::FrontSubstr({ TX("000000000") }, 9 - FractionalLen);
		}
	}

	const i32 days = AtoI32(Tokens[0].c_str());
	const i32 hours = AtoI32(Tokens[1].c_str());
	const i32 minutes = AtoI32(Tokens[2].c_str());
	const i32 seconds = AtoI32(Tokens[3].c_str());
	const i32 fractionNano = AtoI32(Tokens[4].c_str());

	// Max days
	if ((days > floor<date::days>(decmicroseconds::max()).count() -1))
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
	duration = floor<decmicroseconds>(
		            date::days { days }
	      + std::chrono::hours { hours }
	    + std::chrono::minutes { minutes }
	    + std::chrono::seconds { seconds }
	+ std::chrono::nanoseconds { fractionNano });
}

Timespan Timespan::FromHours(i32 hours)
{
	return Timespan{ hours, 0, 0 };
}

Timespan Timespan::FromMinutes(i32 minutes)
{
	return Timespan{ 0, minutes, 0 };
}

Timespan Timespan::FromSeconds(i32 seconds)
{
	return Timespan{ 0, 0, seconds };
}
