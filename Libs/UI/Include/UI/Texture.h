// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "UI/UIImgui.h"


namespace rift::UI
{
	// Creates a texture from an image file. Requires a current OpenGL context.
	// Returns ImTextureID_Invalid when the image can't be loaded.
	ImTextureID LoadTexture(p::StringView path);
	// Releases a texture created by LoadTexture
	void UnloadTexture(ImTextureID textureId);
}    // namespace rift::UI
