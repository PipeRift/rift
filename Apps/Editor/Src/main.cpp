// Copyright 2015-2021 Piperift - All rights reserved

#include "Window.h"

#include <Platform/Platform.h>
#include <imgui.h>


using namespace Rift;

Window window{};

void Init()
{
	window.Init();
}

void Frame()
{
	window.Frame();
}

void Shutdown()
{
	window.Shutdown();
}

void Event(const sapp_event* ev)
{
	window.Event(ev);
}

sapp_desc sokol_main(int argc, char* argv[])
{
	return (sapp_desc){
		.init_cb = Init,
		.frame_cb = Frame,
		.cleanup_cb = Shutdown,
		.event_cb = Event,
		.width = 1200,
		.height = 900,
		.window_title = "Rift",
	};
}
