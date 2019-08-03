// Copyright 2015-2019 Piperift - All rights reserved

#include "Pipeline.h"

#include "Profiler.h"
#include "Core/Misc/DateTime.h"


bool Pipeline::Start()
{
	Log::Init();

	{
		Log::Message("Start-Up");
		ScopedGameZone("Start-Up");

		DateTime::InitializeTime();

		assetManager = Create<AssetManager>(Self());

		context = Create<Context>(Self());
		context->Initialize();
	}

	Shutdown();
	Log::Message("Closing");
	return true;
}
