// Copyright 2015-2019 Piperift - All rights reserved

#include "UIManager.h"
#include <SDL_events.h>

#include "Core/Log.h"
#include "Profiler.h"


void UIManager::Prepare()
{
	Log::Message("UI Start-Up");

	editor = Create<EditorManager>(Self());
}

void UIManager::Tick(float deltaTime)
{
	ScopedGameZone("UI Logic");

	editor->Tick(deltaTime);
}
