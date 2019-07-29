// Copyright 2015-2019 Piperift - All rights reserved

#include "UIManager.h"
#include <SDL_events.h>

#include "Core/Log.h"
#include "Tools/Profiler.h"


void UIManager::Prepare()
{
	Log::Message("UI Start-Up");

#if WITH_EDITOR
	editor = Create<EditorManager>(Self());
#endif
}

void UIManager::OnSDLEvent(SDL_Event* ev)
{
#if WITH_EDITOR
	editor->OnSDLEvent(ev);
#endif
}

void UIManager::Tick(float deltaTime)
{
	ScopedGameZone("UI Logic");

#if WITH_EDITOR
	editor->Tick(deltaTime);
#endif
}
