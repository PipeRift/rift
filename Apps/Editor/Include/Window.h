// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include <Misc/Time.h>
#include <sokol_app.h>
#include <sokol_gfx.h>


using namespace Rift;


class Window
{
	FrameTime frameTime;
	sg_pass_action pass_action;


public:
	Window()
	{
		frameTime.SetFPSCap(60);
	}

	void Init();
	void Frame();
	void Event(const struct sapp_event* ev);
	void Shutdown();

	void Tick(float deltaTime);

	void Close();
};
