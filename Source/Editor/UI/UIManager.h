// Copyright 2015-2019 Piperift - All rights reserved

#pragma once

#include "CoreObject.h"

#include <imgui/imgui.h>

#include "Core/Object/ObjectPtr.h"
#include "Editor/EditorManager.h"


class UIManager : public Object {
	CLASS(UIManager, Object)

#if WITH_EDITOR
	GlobalPtr<EditorManager> editor;
#endif


public:

	UIManager() : Super() {}

	void Prepare();

	void OnSDLEvent(union SDL_Event* ev);

	void Tick(float deltaTime);
};
