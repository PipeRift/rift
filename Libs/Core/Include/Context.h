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
	/** Called to initialize the global context. */
	static void Initialize()
	{
		if (!globalInstance)
		{
			globalInstance = Create<Context>();
		}
	}

	/** Called to manually shutdown the global context. */
	static void Shutdown()
	{
		if (globalInstance)
		{
			globalInstance.Reset();
		}
	}

	Context() : Super(), assetManager{Create<AssetManager>()} {}

	virtual void Construct() override
	{
		Super::Construct();

		Log::Init("Saved/Logs");
		Log::Message("Initialize Context");
	}

	virtual void BeforeDestroy() override
	{
		Super::BeforeDestroy();
		Log::Message("Context has been destroyed");
		Log::Shutdown();
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
