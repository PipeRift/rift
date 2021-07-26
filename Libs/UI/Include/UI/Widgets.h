// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "UI/Style.h"
#include "UI/UIImGui.h"


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


	inline void PushID(StringView id)
	{
		ImGui::PushID(id.data(), id.data() + id.size());
	}

	inline ImGuiID GetID(StringView id)
	{
		return ImGui::GetID(id.data(), id.data() + id.size());
	}

	inline bool InputText(const char* label, char* buf, size_t buf_size,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
	    void* user_data = NULL)
	{
		return ImGui::InputText(label, buf, buf_size, flags, callback, user_data);
	}
	inline bool InputTextMultiline(const char* label, char* buf, size_t buf_size,
	    const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0,
	    ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
	{
		return ImGui::InputTextMultiline(label, buf, buf_size, size, flags, callback, user_data);
	}
	inline bool InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
	    void* user_data = NULL)
	{
		return ImGui::InputTextWithHint(label, hint, buf, buf_size, flags, callback, user_data);
	}

	// ImGui::InputText() with String
	// Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
	bool InputText(const char* label, String& str, ImGuiInputTextFlags flags = 0,
	    ImGuiInputTextCallback callback = NULL, void* userData = NULL);
	bool InputTextMultiline(const char* label, String& str, const ImVec2& size = ImVec2(0, 0),
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
	    void* userData = NULL);
	bool InputTextWithHint(const char* label, const char* hint, String& str,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL,
	    void* userData = NULL);


	static bool Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0)
	{
		const bool value = ImGui::Begin(name, p_open, flags);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 3.f));
		return value;
	}

	static void End()
	{
		ImGui::PopStyleVar();
		ImGui::End();
	}

	ImRect GetWorkRect(v2 desiredSize, bool addhalfItemSpacing = true, v2 extent = v2::Zero());

	bool MutableText(StringView label, String& text);
}    // namespace Rift::UI
