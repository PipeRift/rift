// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include <ctype.h>
#include <wctype.h>

/*-----------------------------------------------------------------------------
	Character type functions.
-----------------------------------------------------------------------------*/

/**
 * Templated literal struct to allow selection of wide or ansi string literals
 * based on the character type provided, and not on compiler switches.
 */
template <typename T> struct TLiteral
{
	static const ANSICHAR  Select(const ANSICHAR  ansi, const WIDECHAR ) { return ansi; }
	static const ANSICHAR* Select(const ANSICHAR* ansi, const WIDECHAR*) { return ansi; }
};

template <> struct TLiteral<WIDECHAR>
{
	static const WIDECHAR  Select(const ANSICHAR,  const WIDECHAR  wide) { return wide; }
	static const WIDECHAR* Select(const ANSICHAR*, const WIDECHAR* wide) { return wide; }
};

#define LITERAL(CharType, StringLiteral) TLiteral<CharType>::Select(StringLiteral, L##StringLiteral)

/**
 * TChar
 * Set of utility functions operating on a single character. The functions
 * are specialized for ANSICHAR and TCHAR character types. You can use the
 * typedefs FChar and FCharAnsi for convenience.
 */

template <typename T, const unsigned int Size>
struct TCharBase
{
	typedef T CharType;

	static const CharType LineFeed				= L'\x000A';
	static const CharType VerticalTab			= L'\x000B';
	static const CharType FormFeed				= L'\x000C';
	static const CharType CarriageReturn		= L'\x000D';
	static const CharType NextLine				= L'\x0085';
	static const CharType LineSeparator			= L'\x2028';
	static const CharType ParagraphSeparator	= L'\x2029';
};

template <typename T>
struct TCharBase<T, 1>
{
	typedef T CharType;

	static const CharType LineFeed			= '\x000A';
	static const CharType VerticalTab		= '\x000B';
	static const CharType FormFeed			= '\x000C';
	static const CharType CarriageReturn	= '\x000D';
	static const CharType NextLine			= '\x0085';
};

template <typename T, const unsigned int Size>
struct LineBreakImplementation
{
	typedef T CharType;
	static inline bool IsLinebreak( CharType c )
	{
		return	c == TCharBase<CharType, Size>::LineFeed			 
			 || c == TCharBase<CharType, Size>::VerticalTab		 
			 || c == TCharBase<CharType, Size>::FormFeed			 
			 || c == TCharBase<CharType, Size>::CarriageReturn	 
			 || c == TCharBase<CharType, Size>::NextLine			 
			 || c == TCharBase<CharType, Size>::LineSeparator		 
			 || c == TCharBase<CharType, Size>::ParagraphSeparator;
	}
};

template <typename T>
struct LineBreakImplementation<T, 1>
{
	typedef T CharType;
	static inline bool IsLinebreak( CharType c )
	{
		return	c == TCharBase<CharType, 1>::LineFeed		 
			 || c == TCharBase<CharType, 1>::VerticalTab	 
			 || c == TCharBase<CharType, 1>::FormFeed		 
			 || c == TCharBase<CharType, 1>::CarriageReturn 
			 || c == TCharBase<CharType, 1>::NextLine	   ;
	}
};

template <typename T>
struct TChar : public TCharBase<T, sizeof(T)>
{
	typedef T CharType;
public:
	static inline CharType ToUpper(CharType c) { return eastl::CharToUpper(c); }
	static inline CharType ToLower(CharType c) { return eastl::CharToLower(c); }
	static inline bool IsUpper(CharType c) { return eastl::upper}
	static inline bool IsLower(CharType c);
	static inline bool IsAlpha(CharType c);
	static inline bool IsGraph(CharType c);
	static inline bool IsPrint(CharType c);
	static inline bool IsPunct(CharType c);

	static inline bool IsAlnum(CharType c);
	static inline bool IsDigit(CharType c);
	static inline bool IsOctDigit(CharType c)
	{
		return c >= '0' && c <= '7';
	}
	static inline bool IsHexDigit(CharType c);
	static inline i32 ConvertCharDigitToInt(CharType c)
	{
		return static_cast<i32>(c) - static_cast<i32>('0');
	}
	static inline bool IsWhitespace(CharType c);
	static inline bool IsIdentifier(CharType c)
	{
		return IsAlnum(c) || IsUnderscore(c);
	}
	static inline bool IsUnderscore(CharType c)		{ return c == LITERAL(CharType, '_'); }

	static inline bool IsLinebreak(CharType c) { return LineBreakImplementation<CharType, sizeof(CharType)>::IsLinebreak(c); }

	static inline i32 StrtoI32(const CharType* str, CharType** end, i32 radix);
};

typedef TChar<TCHAR>    FChar;
typedef TChar<WIDECHAR> FCharWide;
typedef TChar<ANSICHAR> FCharAnsi;

/*-----------------------------------------------------------------------------
	WIDECHAR specialized functions
-----------------------------------------------------------------------------*/
template <> inline bool  TChar<WIDECHAR>::IsUpper(CharType c)							{ return ::iswupper(c) != 0; }
template <> inline bool  TChar<WIDECHAR>::IsLower(CharType c)							{ return ::iswlower(c) != 0; }
template <> inline bool  TChar<WIDECHAR>::IsAlpha(CharType c)							{ return ::iswalpha(c) != 0; }
template <> inline bool  TChar<WIDECHAR>::IsGraph(CharType c)							{ return ::iswgraph(c) != 0; }
template <> inline bool  TChar<WIDECHAR>::IsPrint(CharType c)							{ return ::iswprint(c) != 0; }
template <> inline bool  TChar<WIDECHAR>::IsPunct(CharType c)							{ return ::iswpunct(c) != 0; }
template <> inline bool  TChar<WIDECHAR>::IsAlnum(CharType c)							{ return ::iswalnum(c) != 0; }
template <> inline bool  TChar<WIDECHAR>::IsDigit(CharType c)							{ return ::iswdigit(c) != 0; }
template <> inline bool  TChar<WIDECHAR>::IsHexDigit(CharType c)					   	{ return ::iswxdigit(c) != 0; }
template <> inline bool  TChar<WIDECHAR>::IsWhitespace(CharType c)                      { return ::iswspace(c) != 0; }
template <> inline i32 TChar<WIDECHAR>::StrtoI32(const CharType* str, CharType** end, i32 radix) { return ::wcstol(str, end, radix); }

/*-----------------------------------------------------------------------------
	ANSICHAR specialized functions
-----------------------------------------------------------------------------*/
template <> inline bool  TChar<ANSICHAR>::IsUpper(CharType Char)							{ return ::isupper((unsigned char)Char) != 0; }
template <> inline bool  TChar<ANSICHAR>::IsLower(CharType Char)							{ return ::islower((unsigned char)Char) != 0; }
template <> inline bool  TChar<ANSICHAR>::IsAlpha(CharType Char)							{ return ::isalpha((unsigned char)Char) != 0; }
template <> inline bool  TChar<ANSICHAR>::IsGraph(CharType Char)							{ return ::isgraph((unsigned char)Char) != 0; }
template <> inline bool  TChar<ANSICHAR>::IsPrint(CharType Char)							{ return ::isprint((unsigned char)Char) != 0; }
template <> inline bool  TChar<ANSICHAR>::IsPunct(CharType Char)							{ return ::ispunct((unsigned char)Char) != 0; }
template <> inline bool  TChar<ANSICHAR>::IsAlnum(CharType Char)							{ return ::isalnum((unsigned char)Char) != 0; }
template <> inline bool  TChar<ANSICHAR>::IsDigit(CharType Char)							{ return ::isdigit((unsigned char)Char) != 0; }
template <> inline bool  TChar<ANSICHAR>::IsHexDigit(CharType Char)				    		{ return ::isxdigit((unsigned char)Char) != 0; }
template <> inline bool  TChar<ANSICHAR>::IsWhitespace(CharType Char)                       { return ::isspace((unsigned char)Char) != 0; }
template <> inline i32 TChar<ANSICHAR>::StrtoI32(const CharType* str, CharType** end, i32 radix) { return ::strtol(str, end, radix); }
