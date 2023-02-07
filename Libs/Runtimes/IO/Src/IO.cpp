// Copyright 2015-2023 Piperift - All rights reserved

#include "IO.h"

#include <iostream>


extern "C"
{
	void Print(const char* text)
	{
		std::cout << text;
	}

	void PrintHelloWorld()
	{
		Print("Hello World");
	}
}
