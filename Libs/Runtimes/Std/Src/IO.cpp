// Copyright 2015-2023 Piperift - All rights reserved

#include "IO.h"

#include <cstdio>


extern "C"
{
	void Print(const char* text)
	{
		printf(text);
	}

	void PrintHelloWorld()
	{
		Print("Hello World");
	}
}
