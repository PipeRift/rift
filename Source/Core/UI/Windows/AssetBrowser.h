// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "UI/Window.h"


class AssetBrowser : public Window {
	CLASS(AssetBrowser, Window)

protected:

	virtual void Build() override;

	virtual void Tick(float deltaTime) override;
};

#endif
