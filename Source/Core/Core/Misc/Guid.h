// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreEngine.h"
#include "Core/Strings/Name.h"
#include "Core/Reflection/ClassTraits.h"
#include "Core/Reflection/ReflectionTypeTraits.h"

#include "Crc.h"


/**
 * Enumerates known GUID formats.
 */
enum class EGuidFormats
{
	/**
	 * 32 digits.
	 *
	 * For example: "00000000000000000000000000000000"
	 */
	Digits,

	/**
	 * 32 digits separated by hyphens.
	 *
	 * For example: 00000000-0000-0000-0000-000000000000
	 */
	DigitsWithHyphens,

	/**
	 * 32 digits separated by hyphens and enclosed in braces.
	 *
	 * For example: {00000000-0000-0000-0000-000000000000}
	 */
	DigitsWithHyphensInBraces,

	/**
	 * 32 digits separated by hyphens and enclosed in parentheses.
	 *
	 * For example: (00000000-0000-0000-0000-000000000000)
	 */
	DigitsWithHyphensInParentheses,

	/**
	 * Comma-separated hexadecimal values enclosed in braces.
	 *
	 * For example: {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
	 */
	HexValuesInBraces,

	/**
	 * This format is currently used by the FUniqueObjectGuid class.
	 *
	 * For example: 00000000-00000000-00000000-00000000
	*/
	UniqueObjectGuid,
};


/**
 * Implements a globally unique identifier.
 */
struct Guid
{
public:

	/** Default constructor. */
	Guid()
		: a(0)
		, b(0)
		, c(0)
		, d(0)
	{ }

	/**
	 * Creates and initializes a new GUID from the specified components.
	 *
	 * @param InA The first component.
	 * @param InB The second component.
	 * @param InC The third component.
	 * @param InD The fourth component.
	 */
	Guid(u32 InA, u32 InB, u32 InC, u32 InD)
		: a(InA), b(InB), c(InC), d(InD)
	{ }

public:

	/**
	 * Compares two GUIDs for equality.
	 *
	 * @param X The first GUID to compare.
	 * @param Y The second GUID to compare.
	 * @return true if the GUIDs are equal, false otherwise.
	 */
	friend bool operator==(const Guid& X, const Guid& Y)
	{
		return ((X.a ^ Y.a) | (X.b ^ Y.b) | (X.c ^ Y.c) | (X.d ^ Y.d)) == 0;
	}

	/**
	 * Compares two GUIDs for inequality.
	 *
	 * @param X The first GUID to compare.
	 * @param Y The second GUID to compare.
	 * @return true if the GUIDs are not equal, false otherwise.
	 */
	friend bool operator!=(const Guid& X, const Guid& Y)
	{
		return ((X.a ^ Y.a) | (X.b ^ Y.b) | (X.c ^ Y.c) | (X.d ^ Y.d)) != 0;
	}

	/**
	 * Compares two GUIDs.
	 *
	 * @param X The first GUID to compare.
	 * @param Y The second GUID to compare.
	 * @return true if the first GUID is less than the second one.
	 */
	friend bool operator<(const Guid& X, const Guid& Y)
	{
		return	((X.a < Y.a) ? true : ((X.a > Y.a) ? false :
				((X.b < Y.b) ? true : ((X.b > Y.b) ? false :
				((X.c < Y.c) ? true : ((X.c > Y.c) ? false :
				((X.d < Y.d) ? true : ((X.d > Y.d) ? false : false)))))))); //-V583
	}

	/**
	 * Provides access to the GUIDs components.
	 *
	 * @param Index The index of the component to return (0...3).
	 * @return The component.
	 */
	u32& operator[](u8 Index)
	{
		//checkSlow(Index >= 0);
		//checkSlow(Index < 4);

		switch(Index)
		{
		case 0: return a;
		case 1: return b;
		case 2: return c;
		case 3: return d;
		}

		return a;
	}

	/**
	 * Provides read-only access to the GUIDs components.
	 *
	 * @param Index The index of the component to return (0...3).
	 * @return The component.
	 */
	const u32& operator[](u8 Index) const
	{
		//checkSlow(Index >= 0);
		//checkSlow(Index < 4);

		switch(Index)
		{
		case 0: return a;
		case 1: return b;
		case 2: return c;
		case 3: return d;
		}

		return a;
	}

	bool Serialize(class Archive& Ar, const char* name);

	/**
	* Guid default string conversion.
	*/
	friend String LexToString(const Guid& Value)
	{
		return Value.ToString();
	}

	friend void LexFromString(Guid& Result, const TCHAR* String)
	{
		Guid::Parse(String, Result);
	}

public:

	/**
	 * Invalidates the GUID.
	 *
	 * @see IsValid
	 */
	void Invalidate()
	{
		a = b = c = d = 0;
	}

	/**
	 * Checks whether this GUID is valid or not.
	 *
	 * A GUID that has all its components set to zero is considered invalid.
	 *
	 * @return true if valid, false otherwise.
	 * @see Invalidate
	 */
	bool IsValid() const
	{
		return ((a | b | c | d) != 0);
	}

	/**
	 * Converts this GUID to its string representation.
	 *
	 * @return The string representation.
	 */
	String ToString() const
	{
		return ToString(EGuidFormats::Digits);
	}

	/**
	 * Converts this GUID to its string representation using the specified format.
	 *
	 * @param Format The string format to use.
	 * @return The string representation.
	 */
	String ToString(EGuidFormats Format) const;

public:

	/**
	 * Returns a new GUID.
	 *
	 * @return A new GUID.
	 */
	static Guid New();

	/**
	 * Converts a string to a GUID.
	 *
	 * @param GuidString The string to convert.
	 * @param OutGuid Will contain the parsed GUID.
	 * @return true if the string was converted successfully, false otherwise.
	 * @see ParseExact, ToString
	 */
	static bool Parse(const String& GuidString, Guid& OutGuid);

	/**
	 * Converts a string with the specified format to a GUID.
	 *
	 * @param GuidString The string to convert.
	 * @param Format The string format to parse.
	 * @param OutGuid Will contain the parsed GUID.
	 * @return true if the string was converted successfully, false otherwise.
	 * @see Parse, ToString
	 */
	static bool ParseExact(const String& GuidString, EGuidFormats Format, Guid& OutGuid);

//private:
public:

	/** Holds the first component. */
	u32 a;

	/** Holds the second component. */
	u32 b;

	/** Holds the third component. */
	u32 c;

	/** Holds the fourth component. */
	u32 d;
};

DEFINE_CLASS_TRAITS(Guid,
	HasCustomSerialize = true
);
DECLARE_REFLECTION_TYPE(Guid);

namespace eastl {
	template <>
	struct hash<Guid> {
		size_t operator()(const Guid& k) const
		{
			return Crc::MemCrc32(&k, sizeof(Guid));
		}
	};
};

