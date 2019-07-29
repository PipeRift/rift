// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "CoreObject.h"
#include "CoreTypes.h"
#include <SDL.h>
#include <imgui/imgui.h>
#include <EASTL/queue.h>

#include "Frame.h"
#include "Resources.h"
#include "Commands/RenderCommand.h"


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720


class Renderer : public Object
{
	CLASS(Renderer, Object)


	static const char* glslVersion;

	// Raw ptr to engine. Renderer cant use Ptr<>
	class Engine* engine;
	SDL_Window* window;
	SDL_GLContext gl_context;

	FrameRender render;


public:

	Renderer();

	bool Initialize();

	void PrepareUI();

	void PreTick();

	void Render();
	void RenderUI();

	void SwapWindow();

	virtual void BeforeDestroy() override;

	u32 GetWindowId() const { return window ? SDL_GetWindowID(window) : 0; }
};
