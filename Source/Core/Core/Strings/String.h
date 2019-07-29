// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include <regex>
#include <EASTL/string.h>
#include <EASTL/string_view.h>
#pragma warning(disable : 4996)
#include <EAStdC/EAString.h>

#include "Core/Platform/Platform.h"
#include "Core/Memory/Allocator.h"
#include "Core/Containers/Array.h"
#include "../Math/Math.h"


using String = eastl::basic_string<TCHAR, StringAllocator>;
using StringView = eastl::basic_string_view<TCHAR>;


struct CString {
	template<typename ...Args>
	static String Printf(const TCHAR* format, Args... args) {
		String str;
		str.sprintf(format, eastl::forward<Args>(args)...);
		return eastl::move(str);
	}


	static void ToSentenceCase(const String& str, String& result);

	static String ReplaceCopy(const String& original, const TCHAR searchChar, const TCHAR replacementChar) {
		String result = original;
		Replace(result, searchChar, replacementChar);
		return eastl::move(result);
	}

	static void Replace(String& value, const TCHAR searchChar, const TCHAR replacementChar) {
		eastl::replace(value.begin(), value.end(), searchChar, replacementChar);
	}

	static bool Contains(const String& str, const TCHAR c) {
		return str.find(c) != String::npos;
	}

	static bool Contains(const String& str, const TCHAR* c) {
		return str.find(c) != String::npos;
	}

	static bool Equals(const String& str, const TCHAR* c, size_t size) {
		return eastl::Compare(str.c_str(), c, Math::Min(size, str.length())) == 0;
	}

	static bool Equals(const String& str, const TCHAR c) {
		return str.size() == 1 && str[0] == c;
	}

	/**
	 * Breaks up a delimited string into elements of a string array.
	 *
	 * @param	InArray		The array to fill with the string pieces
	 * @param	pchDelim	The string to delimit on
	 * @param	InCullEmpty	If 1, empty strings are not added to the array
	 *
	 * @return	The number of elements in InArray
	 */
	static i32 ParseIntoArray(const String& str, TArray<String>& OutArray, const TCHAR* pchDelim, bool InCullEmpty = true);

	static FORCEINLINE bool StartsWith(const String& str, const String& subStr) {
		return StartsWith(str, subStr.c_str());
	}
	static FORCEINLINE bool StartsWith(const String& str, const TCHAR* subStr) {
		return str.find(subStr) != std::string::npos;
	}

	static FORCEINLINE bool EndsWith(const String& str, const String& subStr) {
		return subStr.size() <= str.size() && eastl::equal(subStr.rbegin(), subStr.rend(), str.rbegin());
	}
	static FORCEINLINE bool EndsWith(const String& str, const TCHAR* subStr) {
		return EndsWith(str, { subStr });
	}
	static FORCEINLINE bool EndsWith(const String& str, const TCHAR c) {
		return str.size() >= 1 && str.back() == c;
	}

	static void RemoveFromStart(String& str, size_t size) {
		str.erase(0, size);
	}
	static void RemoveFromEnd(String& str, size_t size) {
		str.erase(str.size() - 1 - size, size);
	}

	static i32 Split(const String& str, TArray<String>& tokens, const TCHAR delim)
	{
		std::size_t current, previous = 0;
		current = str.find(delim);
		while (current != std::string::npos) {
			tokens.Add(str.substr(previous, current - previous));
			previous = current + 1;
			current = str.find(delim, previous);
		}
		tokens.Add(str.substr(previous, current - previous));
		return tokens.Size();
	}

	static bool Split(const String& str, String& a, String& b, const TCHAR* delim)
	{
		const size_t pos = str.find(delim);
		if(pos != String::npos)
		{
			a = str.substr(0, pos);
			b = str.substr(pos, str.size() - pos);
			return true;
		}
		return false;
	}

	static String FrontSubstr(const String& str, i32 size) {
		return str.substr(0, size);
	}

	static String BackSubstr(const String& str, i32 size);

	/* Unneeded
	static FORCEINLINE uint32 StrtoU32(const String& str) {
		return EA::StdC::AtoU32(str.c_str());
	}

	static FORCEINLINE int32 StrtoI32(const String& str) {
		return EA::StdC::AtoI32(str.c_str());
	}*/

	static FORCEINLINE bool IsNumeric(const String& str) { return IsNumeric(str.data()); }
	static bool IsNumeric(const TCHAR* Str);

	static String ParseMemorySize(size_t size);

private:

	// #NOTE: EASTL doesn't support codecvt and wstring conversion yet
	//static std::wstring_convert<std::codecvt_utf8<TCHAR>, TCHAR, StringAllocator, StringAllocator> converter;

public:

	//static String ToString(const UTF8String& str) { return TX(""); /*converter.from_bytes(str);*/ }
	//static UTF8String ToUTF8(const String& str) { return ""; /*converter.to_bytes(str);*/ }
};

using Regex = std::basic_regex<TCHAR>;


namespace eastl
{
	template <>
	struct hash<String>
	{
		size_t operator()(const String& x) const
		{
			const TCHAR* p = (const TCHAR*)x.c_str(); // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like String hash.
			while ((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};


	/// to_String
	///
	/// Converts integral types to an eastl::String with the same content that sprintf produces.  The following
	/// implementation provides a type safe conversion mechanism which avoids the common bugs associated with sprintf
	/// style format Strings.
	///
	/// http://en.cppreference.com/w/cpp/String/basic_String/to_String
	///
	inline String to_String(int value)
	{
		return String(String::CtorSprintf(), TX("%d"), value);
	}
	inline String to_String(long value)
	{
		return String(String::CtorSprintf(), TX("%ld"), value);
	}
	inline String to_String(long long value)
	{
		return String(String::CtorSprintf(), TX("%lld"), value);
	}
	inline String to_String(unsigned value)
	{
		return String(String::CtorSprintf(), TX("%u"), value);
	}
	inline String to_String(unsigned long value)
	{
		return String(String::CtorSprintf(), TX("%lu"), value);
	}
	inline String to_String(unsigned long long value)
	{
		return String(String::CtorSprintf(), TX("%llu"), value);
	}
	inline String to_String(float value)
	{
		return String(String::CtorSprintf(), TX("%f"), value);
	}
	inline String to_String(double value)
	{
		return String(String::CtorSprintf(), TX("%f"), value);
	}
	inline String to_String(long double value)
	{
		return String(String::CtorSprintf(), TX("%Lf"), value);
	}



	/// user defined literals
	///
	/// Converts a character array literal to a basic_string.
	///
	/// Example:
	///   String s = "abcdef"s;
	///
	/// http://en.cppreference.com/w/cpp/String/basic_String/operator%22%22s
	///
	#if EASTL_USER_LITERALS_ENABLED && EASTL_INLINE_NAMESPACES_ENABLED
	EA_DISABLE_VC_WARNING(4455) // disable warning C4455: literal suffix identifiers that do not start with an underscore are reserved
	inline namespace literals
	{
		inline namespace string_literals
		{
		}
	}
	EA_RESTORE_VC_WARNING()  // warning: 4455
	#endif
} // namespace eastl
