// Copyright 2015-2019 Piperift - All rights reserved

#include "String.h"
#include "Core/Misc/Char.h"
#include "Core/Math/Math.h"

void CString::ToSentenceCase(const String& str, String& result)
{
	if (!str.empty())
	{
		//static const std::regex wordToCapital("\b[a-z]");
		static const Regex spaceCamelCase(TX("([a-zA-Z])(?=[A-Z0-9])"));

		result = String{ std::regex_replace(str.c_str(), spaceCamelCase, TX("$& ")).c_str() };
		result[0] = (char)::toupper(result[0]);
	}
	else
	{
		result = TX("");
	}
}

i32 CString::ParseIntoArray(const String& str, TArray<String>& OutArray, const TCHAR* pchDelim, bool InCullEmpty /*= true*/)
{
	return 0;
}

String CString::BackSubstr(const String& str, i32 size)
{
	return str.substr(Math::Max((SIZE_T)0u, SIZE_T(str.size() - size)), size);
}

bool CString::IsNumeric(const TCHAR* Str)
{
	if (*Str == TX('-') || *Str == TX('+'))
	{
		Str++;
	}

	bool bHasDot = false;
	while (*Str != TX('\0'))
	{
		if (*Str == TX('.'))
		{
			if (bHasDot)
			{
				return false;
			}
			bHasDot = true;
		}
		else if (!FChar::IsDigit(*Str))
		{
			return false;
		}

		++Str;
	}

	return true;
}

String CString::ParseMemorySize(size_t size)
{
	if (size <= 0)
		return "0 B";

	static const char* sizes[] { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };

	const double scaleD = Math::Log(double(size), 1024.l);
	const size_t scale = Math::FloorToI64(scaleD);
	const size_t finalSize = size / Math::Pow<size_t>(1024, scale);

	return CString::Printf("%i %s", finalSize, sizes[scale]);
}

//const std::wstring_convert<std::codecvt_utf8<WIDECHAR>, WIDECHAR, StringAllocator, StringAllocator> CString::converter {};
