// Copyright 2015-2019 Piperift - All rights reserved

#include <cstdio>
#include <iostream>

#include "Application.h"
#include "Core/Log.h"

int main()
{
	GlobalPtr<Application> app{ Create<Application>() };
	app->Start();
	return true;
}
