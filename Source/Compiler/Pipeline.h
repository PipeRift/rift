// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"

#include "Context.h"
#include "Core/MultiThreading.h"


class Pipeline : public Object
{
	CLASS(Pipeline, Object)

	GlobalPtr<Context> context;
	GlobalPtr<AssetManager> assetManager;

	TaskSystem taskSystem;

public:

	Pipeline() : Super() {}


	/** Begin Object interface */
protected:
	virtual void BeforeDestroy() override { Shutdown(); }
public:
	virtual Ptr<Context> GetContext() const override { return context; }
	/** End Object interface */


	bool Start();

	void Shutdown() {}

	Ptr<AssetManager> GetAssetManager() const { return assetManager; }

	TaskSystem& Tasks() { return taskSystem; }
};
