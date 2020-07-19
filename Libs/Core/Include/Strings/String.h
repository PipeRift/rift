// Copyright 2015-2020 Piperift - All rights reserved

#pragma once


#include "Containers/Array.h"
#include "Containers/Tuples.h"
#include "Math/Math.h"
#include "Memory/Allocator.h"
#include "Misc/Hash.h"
#include "Platform/Platform.h"

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <charconv>
#include <regex>
#include <string>
#include <string_view>


#pragma warning(disable : 4996)


namespace VCLang
{
	using String = std::basic_string<TCHAR>;
	using StringView = std::basic_string_view<TCHAR>;

	struct CString
	{
		template <typename... Args>
		static String Format(StringView format, Args... args)
		{
			return fmt::format(format, std::forward<Args>(args)...);
		}

		template <typename... Args>
		static void FormatTo(String& buffer, StringView format, Args... args)
		{
			fmt::format_to(std::back_inserter(buffer), format, std::forward<Args>(args)...);
		}

		static void ToSentenceCase(const String& str, String& result);

		static String ReplaceCopy(
			const String& original, const TCHAR searchChar, const TCHAR replacementChar)
		{
			String result = original;
			Replace(result, searchChar, replacementChar);
			return result;
		}

		static void Replace(String& value, const TCHAR searchChar, const TCHAR replacementChar)
		{
			std::replace(value.begin(), value.end(), searchChar, replacementChar);
		}

		static bool Contains(StringView str, const TCHAR c)
		{
			return str.find(c) != String::npos;
		}

		static bool Contains(StringView str, StringView subStr)
		{
			return str.find(subStr) != String::npos;
		}

		static constexpr bool Equals(StringView one, StringView other)
		{
			return one.size() == other.size() && std::equal(one.begin(), one.end(), other.begin());
		}

		static constexpr bool Equals(StringView str, const TCHAR c)
		{
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
		static i32 ParseIntoArray(const String& str, TArray<String>& OutArray,
			const TCHAR* pchDelim, bool InCullEmpty = true);

		static constexpr bool StartsWith(StringView str, StringView subStr)
		{
			return str.size() >= subStr.size() &&
				   std::equal(subStr.begin(), subStr.end(), str.begin()) == 0;
		}

		static constexpr bool EndsWith(StringView str, StringView subStr)
		{
			return str.size() >= subStr.size() &&
				   std::equal(subStr.rbegin(), subStr.rend(), str.rbegin());
		}

		static constexpr bool EndsWith(StringView str, const TCHAR c)
		{
			return str.size() >= 1 && str.back() == c;
		}

		static void RemoveFromStart(String& str, size_t size)
		{
			str.erase(0, size);
		}
		static void RemoveFromEnd(String& str, size_t size)
		{
			str.erase(str.size() - 1 - size, size);
		}

		static i32 Split(const String& str, TArray<String>& tokens, const TCHAR delim)
		{
			std::size_t current, previous = 0;
			current = str.find(delim);
			while (current != std::string::npos)
			{
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
			if (pos != String::npos)
			{
				a = str.substr(0, pos);
				b = str.substr(pos, str.size() - pos);
				return true;
			}
			return false;
		}

		static String FrontSubstr(const String& str, i32 size)
		{
			return str.substr(0, size);
		}

		static String BackSubstr(const String& str, i32 size);

		static TOptional<u32> ToU32(StringView str)
		{
			u32 val;
			if (std::from_chars(str.data(), str.data() + str.size(), val).ec != std::errc())
			{
				return val;
			}
			return {};
		}

		static TOptional<i32> ToI32(StringView str)
		{
			i32 val;
			if (std::from_chars(str.data(), str.data() + str.size(), val).ec != std::errc())
			{
				return val;
			}
			return {};
		}

		static TOptional<u32> ToU32(const char* str)
		{
			if (str)
			{
				return std::strtoul(str, nullptr, 0);
			}
			return {};
		}

		static TOptional<i32> ToI32(const char* str)
		{
			if (str)
			{
				return std::atoi(str);
			}
			return {};
		}

		static bool IsNumeric(const String& str)
		{
			return IsNumeric(str.data());
		}
		static bool IsNumeric(const TCHAR* Str);

		static String ParseMemorySize(size_t size);

	private:
		// #NOTE: EASTL doesn't support codecvt and wstring conversion yet
		// static std::wstring_convert<std::codecvt_utf8<TCHAR>, TCHAR, StringAllocator,
		// StringAllocator> converter;

	public:
		// static String ToString(const UTF8String& str) { return TX("");
		// /*converter.from_bytes(str);*/ } static UTF8String ToUTF8(const String& str) { return "";
		// /*converter.to_bytes(str);*/ }
	};

	using Regex = std::basic_regex<TCHAR>;


	template <>
	struct Hash<String>
	{
		size_t operator()(const String& str) const
		{
			return HashBytes(str.data(), str.size());
			// To consider: limit p to at most 256 chars.
			/*const TCHAR* p = (const TCHAR*) x.c_str();
			unsigned int c, result = 2166136261U;	 // We implement an FNV-like String hash.
			while ((c = *p++) != 0)					 // Using '!=' disables compiler warnings.
			{
				result = (result * 16777619) ^ c;
			}
			return (size_t) result;*/
		}
	};

	template <>
	struct Hash<StringView>
	{
		size_t operator()(const StringView& str) const
		{
			return HashBytes(str.data(), str.size());
			// To consider: limit p to at most 256 chars.
			/*const TCHAR* p = (const TCHAR*) x.data();
			unsigned int c, result = 2166136261U;	 // We implement an FNV-like String hash.
			while ((c = *p++) != 0)					 // Using '!=' disables compiler warnings.
			{
				result = (result * 16777619) ^ c;
			}
			return size_t(result);*/
		}
	};
}	 // namespace VCLang
