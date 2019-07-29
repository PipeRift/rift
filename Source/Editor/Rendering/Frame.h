// © 2019 Miguel Fernández Arce - All rights reserved

#pragma once

#include "CoreEngine.h"

#include <EASTL/queue.h>
#include <EASTL/shared_ptr.h>
#include <taskflow/taskflow.hpp>

#include "Core/Misc/DateTime.h"
#include "Commands/RenderCommand.h"
#include "Data/TextureData.h"
#include "Resources.h"
#include "Data/LightData.h"


struct Frame
{
private:
	static u16 idCounter;
	u16 id;

public:

	DateTime time;

	TArray<eastl::unique_ptr<RenderCommand>> commands;


	Frame() { Reset(); }
	Frame(Frame&& other) = default;
	Frame& operator=(Frame&& other) = default;


	void ScheduleCommand(eastl::unique_ptr<RenderCommand>&& command) {
		commands.Add(MoveTemp(command));
	}

	void ExecuteCommands(struct FrameRender& render);

	void Reset() {
		id = idCounter++;
		time = DateTime::UtcNow();
		commands.Empty(false);
	}

	u16 Id() const { return id; }
};


// Where all render data is stored. Gets reused every frame.
struct FrameRender
{
	v2_u32 viewportSize;
	CameraData camera;

	LightingRender lighting;
	Resources resources;

	FrameRender()
		: viewportSize{}
		, camera {}
		, lighting{}
		, resources{}
	{}

	void NewFrame(v2_u32 inViewportSize) {
		ZoneScopedNC("NewFrame", 0x94d145);

		viewportSize = inViewportSize;
		lighting.directionals.Empty();
		lighting.points.Empty();
	}

	v2_u32 GetRenderSize() const { return viewportSize; }

	FORCEINLINE CameraData& Camera() { return camera; }
	FORCEINLINE const CameraData& Camera() const { return camera; }
};

