// © 2019 Miguel Fernández Arce - All rights reserved

#pragma once

#include "CoreEngine.h"

#include <EASTL/queue.h>
#include <EASTL/shared_ptr.h>
#include <taskflow/taskflow.hpp>

#include "Core/Misc/DateTime.h"


struct Frame
{
private:
	static u16 idCounter;
	u16 id;

public:

	DateTime time;


	Frame() { Reset(); }
	Frame(Frame&& other) = default;
	Frame& operator=(Frame&& other) = default;

	void Reset() {
		id = idCounter++;
		time = DateTime::UtcNow();
	}

	u16 Id() const { return id; }
};


// Where all render data is stored. Gets reused every frame.
struct FrameRender
{
	v2_u32 viewportSize;

	FrameRender()
		: viewportSize{}
	{}

	void NewFrame(v2_u32 inViewportSize) {
		ZoneScopedNC("NewFrame", 0x94d145);

		viewportSize = inViewportSize;
	}

	v2_u32 GetRenderSize() const { return viewportSize; }
};

