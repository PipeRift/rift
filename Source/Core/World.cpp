// Copyright 2015-2019 Piperift - All rights reserved

#include "World.h"
#include "../Tools/Profiler.h"
#include "Core/Engine.h"


Broadcast<Ptr<World>> World::Delegates::onBeginPlay{};
Broadcast<Ptr<World>> World::Delegates::onEndPlay{};


void World::Initialize()
{
	Log::Message("World Start-Up");

	ecs = Create<ECSManager>(Self());

	LoadScene({ "3d_physics.meta" });

#if WITH_EDITOR
	worldType = EWorldType::Editor;
	ecs->Initialize();
#else
	// BeginPlay straight away
	worldType = EWorldType::Standalone;
	ecs->Initialize();
	BeginPlay();
#endif
}

void World::BeginPlay()
{
	if (IsPIE())
		return;

	if (IsEditor())
	{
		// Store current scene in memory for EndPlay restore
		scene->SaveScene(Self<World>());
		worldType = EWorldType::PIE;
	}
	else
	{
		worldType = EWorldType::Standalone;
	}

	ScopedGameZone("Begin Play");
	Log::Message("Begin Play");

	ecs->BeginPlay();

	Delegates::onBeginPlay.DoBroadcast(Self<World>());
}

void World::Tick(float deltaTime)
{
	ScopedGameZone("World");

	ecs->Tick(deltaTime);
}

void World::EndPlay()
{
	ScopedGameZone("End Play");
	Log::Message("End Play");

	ecs->EndPlay();

	if (IsPIE())
	{
		worldType = EWorldType::Editor;
		// Restore scene like it was before
		scene->ApplyScene(Self<World>());

		Delegates::onEndPlay.DoBroadcast(Self<World>());
	}
	else
	{
		worldType = EWorldType::EndingPlay;

		Delegates::onEndPlay.DoBroadcast(Self<World>());
		GEngine->Shutdown();
	}
}

void World::Shutdown()
{
	if (IsPIE() || IsStandalone())
	{
		EndPlay();
	}

	ecs->Shutdown();
}

bool World::LoadScene(const TAssetPtr<Scene>& inScene)
{
	if (inScene.IsNull())
	{
		return false;
	}

	// Load if not already
	inScene.Load();

	if (!inScene.IsValid())
	{
		return false;
	}

	scene = inScene;
	return scene->ApplyScene(Self<World>());
}
