// Copyright 2015-2024 Piperift - All rights reserved

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
