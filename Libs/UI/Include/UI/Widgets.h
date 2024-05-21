// Copyright 2015-2024 Piperift - All rights reserved

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
		p::v2 size{};
		float rate = 1.f / 24.f;
		// Number of frames in each row
		p::TArray<p::u32> numFrames{};

		p::v2_u32 currentFrame{};
		float currentFrameRemainingTime = 0.f;


		void SetAnimation(p::u32 id);
		void NextFrame(float deltaTime);

		p::v2 GetUV() const;
	};

	static bool SpriteButton(AnimatedSprite& sprite, p::i32 framePadding,
	    const p::LinearColor& bgColor, const p::LinearColor& tintColor);

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
	bool InputTextWithHint(const char* label, const char* hint, p::String& str,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
	    void* userData = nullptr);

	static void BeginOuterStyle()
	{
		p::LinearColor titleColor = UI::GetNeutralColor(0);
		UI::PushStyleColor(ImGuiCol_TitleBg, titleColor);
		UI::PushStyleColor(ImGuiCol_TitleBgActive, titleColor);
		UI::PushStyleColor(ImGuiCol_TitleBgCollapsed, UI::ToDisabled(titleColor));

		p::LinearColor tabColorActive = UI::GetNeutralColor(1);
		p::LinearColor tabColor       = UI::ToDisabled(tabColorActive);
		UI::PushStyleColor(ImGuiCol_Tab, tabColor);
		UI::PushStyleColor(ImGuiCol_TabActive, tabColorActive);
		UI::PushStyleColor(ImGuiCol_TabUnfocused, tabColor);
		UI::PushStyleColor(ImGuiCol_TabUnfocusedActive, tabColorActive);
		UI::PushStyleColor(ImGuiCol_TabHovered, UI::ToHovered(tabColorActive));
		UI::PushTextColor(UI::GetNeutralTextColor(1));
	}
	static void BeginInnerStyle()
	{
		UI::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.f, 3.f));
	}
	static void EndOuterStyle()
	{
		UI::PopTextColor();
		UI::PopStyleColor(8);
	}
	static void EndInnerStyle()
	{
		UI::PopStyleVar();
	}

	static bool Begin(const char* name, bool* pOpen = nullptr, ImGuiWindowFlags flags = 0)
	{
		BeginOuterStyle();
		const bool value = ImGui::Begin(name, pOpen, flags);
		BeginInnerStyle();
		return value;
	}

	static void End()
	{
		EndInnerStyle();
		ImGui::End();
		EndOuterStyle();
	}

	ImRect GetWorkRect(
	    p::v2 desiredSize, bool addhalfItemSpacing = true, p::v2 extent = p::v2::Zero());

	bool MutableText(p::StringView label, p::String& text, ImGuiInputTextFlags flags = 0);

	void HelpTooltip(p::StringView text, float delay = 1.f);
	void HelpMarker(p::StringView text);

	bool DrawFilterWithHint(ImGuiTextFilter& filter, const char* label = "Filter (inc,-exc)",
	    const char* hint = "...", float width = 0.0f);

	bool CollapsingHeaderWithButton(p::StringView label, ImGuiTreeNodeFlags flags,
	    bool& buttonClicked, p::StringView buttonLabel, p::v2 buttonSize = p::v2(18.f, 14.f));
}    // namespace rift::UI
