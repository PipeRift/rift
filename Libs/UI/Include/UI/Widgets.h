// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "UI/UI.h"


namespace Rift::UI
{
	/**
	 * Tracks an animation from a texture.
	 * Each row is one animation
	 */
	struct AnimatedSprite
	{
		ImTextureID textureId;
		v2 size{};
		float rate = 1.f / 24.f;
		// Number of frames in each row
		TArray<u32> numFrames{};

		v2_u32 currentFrame{};
		float currentFrameRemainingTime = 0.f;


		void SetAnimation(u32 id);
		void NextFrame(float deltaTime);

		v2 GetUV() const;
	};

	static bool SpriteButton(AnimatedSprite& sprite, i32 framePadding, const LinearColor& bgColor,
	    const LinearColor& tintColor);

	// ImGui::InputText() with String
	// Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
	bool InputText(const char* label, String* str, ImGuiInputTextFlags flags = 0,
	    ImGuiInputTextCallback callback = NULL, void* userData = NULL);
	bool InputTextMultiline(const char* label, String* str, const ImVec2& size = ImVec2(0, 0),
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
	    void* userData = NULL);
	bool InputTextWithHint(const char* label, const char* hint, String* str,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
	    void* userData = NULL);
}    // namespace Rift::UI
