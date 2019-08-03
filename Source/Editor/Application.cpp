// Copyright 2015-2019 Piperift - All rights reserved

#include "Application.h"

#include <SDL_image.h>
#include "Profiler.h"


bool Application::Start()
{
	Log::Init();

	{
		Log::Message("Start-Up");
		ScopedGameZone("Start-Up");

		// Setup SDL
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
		{
			Log::Error("Could not initialize SDL.");
			return false;
		}

		u32 imgFlags = IMG_INIT_PNG;
		if (!(IMG_Init(imgFlags) & imgFlags))
		{
			Log::Error("Could not initialize SDL Img library.");
			return false;
		}

		DateTime::InitializeTime();

		renderer = Create<Renderer>(Self());
		if (!renderer->Initialize())
			return false;

		input = Create<InputManager>(Self());
		assetManager = Create<AssetManager>(Self());

		context = Create<Context>(Self());
		context->Initialize();

		ui = Create<UIManager>(Self());
		ui->Prepare();
	}

	frameTime = {};
	//frameTime.SetFPSCap(60);

	Log::Message("Start Loop");
	TaskFlow frameTasks = Tasks().CreateMainFlow();
	bool bFinish = false;
	while (!bFinish)
	{
		Loop(frameTasks, bFinish);
	}

	Shutdown();
	Log::Message("Closing");
	return true;
}

void Application::Loop(TaskFlow& frameTasks, bool& bFinish)
{
	frameTime.Tick();

	// Swap render buffers and reset game buffer
	{
		ScopedGraphicsZone("Swap Buffers");
		SwitchFrameBuffer();
		GetGameFrame().Reset();
	}

	// Pre frame render setup
	renderer->PreTick();

	// Receive Input
	u32 windowId = renderer->GetWindowId();
	bFinish = input->Tick(frameTime.GetDeltaTime(), windowId);

	// Start game thread
	Task gameTick = frameTasks.emplace([this]() {
		ScopedGameZone("Tick");
	});
	frameTasks.dispatch();

	// Do render on main thread while game executes on another
	renderer->Render();

	frameTasks.wait_for_all();

	// UI is rendered on the same frame game does
	ui->Tick(frameTime.GetDeltaTime());
	renderer->RenderUI();

	renderer->SwapWindow();

	frameTime.PostTick();

	FrameMark;
}
