// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#if WITH_EDITOR

#include "Editor/EditorWindow.h"


class AssetBrowser : public EditorWindow {
	CLASS(AssetBrowser, EditorWindow)

protected:

	virtual void Build() override;

	virtual void Tick(float deltaTime) override;
};

#endif
