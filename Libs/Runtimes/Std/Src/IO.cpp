// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "IO.h"

#include <cstdio>


extern "C"
{
	void Print(const char* text)
	{
		printf("%s", text);
	}

	void PrintHelloWorld()
	{
		Print("Hello World");
	}
}
