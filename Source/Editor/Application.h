// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"
#include "Rendering/Renderer.h"

#include "Context.h"
#include "Core/Input/InputManager.h"
#include "Core/MultiThreading.h"
#include "Core/Misc/Time.h"

#include "Editor/UIManager.h"
#include "Rendering/Renderer.h"
#include "Rendering/Frame.h"


class Application : public Object
{
	CLASS(Application, Object)

	FrameTime frameTime;

	GlobalPtr<Context> context;

	GlobalPtr<InputManager> input;
	GlobalPtr<AssetManager> assetManager;
	GlobalPtr<UIManager> ui;
	GlobalPtr<Renderer> renderer;

	Frame frameBuffers[2];
	u32 gameFrameId = 1;

	TaskSystem taskSystem;

public:

	Application() : Super() {}


	/** Begin Object interface */
protected:
	virtual void BeforeDestroy() override { Shutdown(); }
public:
	virtual Ptr<Context> GetContext() const override { return context; }
	/** End Object interface */

private:

	bool Start();

	void Loop(TaskFlow& frameTasks, bool& bFinish);

public:

	void Shutdown() {
		SDL_Quit();
	}

	Ptr<InputManager> GetInput()        const { return input; }
	Ptr<AssetManager> GetAssetManager() const { return assetManager; }
	Ptr<Renderer>     GetRenderer()     const { return renderer; }

	TaskSystem& Tasks() { return taskSystem; }

	/** Frame being prepared on game thread */
	Frame& GetGameFrame() { return frameBuffers[gameFrameId]; }
	/** Frame being rendered on graphics thread */
	Frame& GetRenderFrame() { return frameBuffers[(gameFrameId + 1) % 2]; }

private:

	void SwitchFrameBuffer() { gameFrameId = (gameFrameId + 1) % 2; }
};
