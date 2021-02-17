// Copyright 2015-2021 Piperift - All rights reserved

#include "Window.h"

#include <Context.h>

int main(int, char**)
{
	Rift::Context::Initialize();
	Window window{};
	return window.Run();
}
