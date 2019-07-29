// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"
#include "Rendering/Renderer.h"

#include "World.h"
#include "Rendering/Frame.h"
#include "Input/InputManager.h"
#include "UI/UIManager.h"
#include "Rendering/Renderer.h"
#include "Misc/Time.h"
#include "MultiThreading.h"


class Engine : public Object
{
	CLASS(Engine, Object)

	static Ptr<Engine> globalEngine;

	FrameTime frameTime;

	GlobalPtr<World> world;

	GlobalPtr<InputManager> input;
	GlobalPtr<AssetManager> assetManager;
	GlobalPtr<UIManager> ui;
	GlobalPtr<Renderer> renderer;

	Frame frameBuffers[2];
	u32 gameFrameId = 1;

	TaskSystem taskSystem;

public:

	Engine() : Super() {}


	/** Begin Object interface */
protected:
	virtual void BeforeDestroy() override { Shutdown(); }
public:
	virtual Ptr<World> GetWorld() const override { return world; }
	/** End Object interface */

private:

	bool Start();

	void Loop(TaskFlow& frameTasks, bool& bFinish);

public:

	void Shutdown() {
		world->Shutdown();
		SDL_Quit();
	}

	Ptr<InputManager> GetInput()        const { return input; }
	Ptr<AssetManager> GetAssetManager() const { return assetManager; }
	Ptr<Renderer>     GetRenderer()     const { return renderer; }

	TaskSystem& Tasks() { return taskSystem; }

	/** STATIC */

	static bool StartEngine()
	{
		GlobalPtr<Engine> engine{ Create<Engine>() };
		globalEngine = engine.AsPtr();

		return engine->Start();
	}

	static Ptr<Engine> GetEngine() { return globalEngine; }


	/** Frame being prepared on game thread */
	Frame& GetGameFrame() { return frameBuffers[gameFrameId]; }
	/** Frame being rendered on graphics thread */
	Frame& GetRenderFrame() { return frameBuffers[(gameFrameId + 1) % 2]; }

	template<typename Command, typename ...Args>
	void QueueRenderCommand(Args... args)
	{
		static_assert(eastl::is_base_of<RenderCommand, Command>::value, "Command type must inherit RenderCommand");

		GetGameFrame().ScheduleCommand(
			eastl::make_unique<Command>(eastl::forward<Args>(args)...)
		);
	}

private:

	void SwitchFrameBuffer() { gameFrameId = (gameFrameId + 1) % 2; }
};

#define GEngine Engine::GetEngine()


template<typename Command, typename ...Args>
FORCEINLINE void QueueRenderCommand(Args... args)
{
	GEngine->QueueRenderCommand<Command>(eastl::forward<Args>(args)...);
}