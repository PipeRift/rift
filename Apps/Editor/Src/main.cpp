// Copyright 2015-2021 Piperift - All rights reserved

#include "Window.h"

#include <Context.h>
#include <Platform/Platform.h>
#include <imgui.h>


using namespace Rift;

void Init()
{
	Context::Initialize();
	Window::Get().Init();
}

void Frame()
{
	Window::Get().Frame();
}

void Shutdown()
{
	Window::Get().Shutdown();
}

void Event(const sapp_event* ev)
{
	Window::Get().Event(ev);
}

sapp_desc sokol_main(int argc, char* argv[])
{
	sapp_desc desc{};
	desc.init_cb      = Init;
	desc.frame_cb     = Frame;
	desc.cleanup_cb   = Shutdown;
	desc.event_cb     = Event;
	desc.width        = 1200;
	desc.height       = 900;
	desc.window_title = "Rift";
	return Move(desc);
}
