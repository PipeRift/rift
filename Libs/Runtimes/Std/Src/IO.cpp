// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <Std/IO.h>

#include <cstdio>


extern "C"
{
	void Print(StringView text)
	{
		const p::StringView view{text.data, text.size};
		if (!view.empty())
		{
			std::fwrite(view.data(), 1, view.size(), stdout);
		}
	}

	void PrintLine(StringView text)
	{
		Print(text);
		std::fputc('\n', stdout);
	}
}
