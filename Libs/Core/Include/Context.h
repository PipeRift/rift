// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/AssetManager.h"
#include "Assets/AssetPtr.h"
#include "CoreObject.h"
#include "Events/Broadcast.h"



class CORE_API Context : public Object
{
	CLASS(Context, Object)

private:
	GlobalPtr<AssetManager> assetManager;

	static GlobalPtr<Context> globalInstance;


public:
	Context() : Super(), assetManager{Create<AssetManager>()} {}

	virtual void BeginDestroy()
	{
		Log::Shutdown();
	}

	static void Initialize()
	{
		Log::Init("Saved/Logs");
		globalInstance = Create<Context>();
	}

	Ptr<AssetManager> GetAssetManager()
	{
		return assetManager;
	}

	static Ptr<Context> Get()
	{
		assert(globalInstance && "Context is not initilized! Call Context::Initialize().");
		return globalInstance;
	}
};
