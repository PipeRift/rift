// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "UI/Style.h"
#include "UI/UIImgui.h"


namespace rift::UI
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

	inline bool InputText(const char* label, char* buf, size_t buf_size,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
	    void* user_data = nullptr)
	{
		return ImGui::InputText(label, buf, buf_size, flags, callback, user_data);
	}
	inline bool InputTextMultiline(const char* label, char* buf, size_t buf_size,
	    const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0,
	    ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr)
	{
		return ImGui::InputTextMultiline(label, buf, buf_size, size, flags, callback, user_data);
	}
	inline bool InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
	    void* user_data = nullptr)
	{
		return ImGui::InputTextWithHint(label, hint, buf, buf_size, flags, callback, user_data);
	}

	// ImGui::InputText() with String
	// Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
	bool InputText(const char* label, p::String& str, ImGuiInputTextFlags flags = 0,
	    ImGuiInputTextCallback callback = nullptr, void* userData = nullptr);
	bool InputTextMultiline(const char* label, p::String& str, const ImVec2& size = ImVec2(0, 0),
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
	    void* userData = nullptr);
	bool InputTextWithHint(const char* label, const char* hint, String& str,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
	    void* userData = nullptr);


	static bool Begin(const char* name, bool* pOpen = nullptr, ImGuiWindowFlags flags = 0)
	{
		LinearColor titleColor = UI::GetNeutralColor(0);
		UI::PushStyleColor(ImGuiCol_TitleBg, titleColor);
		UI::PushStyleColor(ImGuiCol_TitleBgActive, titleColor);
		UI::PushStyleColor(ImGuiCol_TitleBgCollapsed, UI::Disabled(titleColor));

		LinearColor tabColorActive = UI::GetNeutralColor(1);
		LinearColor tabColor       = UI::Disabled(tabColorActive);
		UI::PushStyleColor(ImGuiCol_Tab, tabColor);
		UI::PushStyleColor(ImGuiCol_TabActive, tabColorActive);
		UI::PushStyleColor(ImGuiCol_TabUnfocused, tabColor);
		UI::PushStyleColor(ImGuiCol_TabUnfocusedActive, tabColorActive);
		UI::PushStyleColor(ImGuiCol_TabHovered, UI::Hovered(tabColorActive));
		UI::PushTextColor(UI::GetNeutralTextColor(1));

		const bool value = ImGui::Begin(name, pOpen, flags);

		UI::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 3.f));
		return value;
	}

	static void End()
	{
		UI::PopStyleVar();
		ImGui::End();
		UI::PopTextColor();
		UI::PopStyleColor(5);
		UI::PopStyleColor(3);
	}

	ImRect GetWorkRect(v2 desiredSize, bool addhalfItemSpacing = true, v2 extent = v2::Zero());

	bool MutableText(p::StringView label, p::String& text, ImGuiInputTextFlags flags = 0);

	void HelpTooltip(p::StringView text, float delay = 1.f);
	void HelpMarker(p::StringView text);
}    // namespace rift::UI
