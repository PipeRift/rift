// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "UI/Style.h"
#include "UI/UIImgui.h"


namespace rift::UI
{
	/**
	 * Tracks an animation from a sprite sheet.
	 * The sheet is a grid of 'columns' equally wide frames, and each row is one animation.
	 * numFrames holds how many frames of each row are actually used.
	 */
	struct AnimatedSprite
	{
		AnimatedSprite() = default;
		~AnimatedSprite();
		// Sprites own their texture, so they can't be copied
		AnimatedSprite(const AnimatedSprite&)            = delete;
		AnimatedSprite& operator=(const AnimatedSprite&) = delete;
		AnimatedSprite(AnimatedSprite&& other) noexcept;
		AnimatedSprite& operator=(AnimatedSprite&& other) noexcept;

		// Loads a sprite sheet from an image file. Every row holds the same number of frames
		bool Load(p::StringView path, p::u32 framesPerRow, p::u32 animations = 1);
		// Releases the texture and resets the sprite
		void Unload();

		p::v2 size{};                   // Size of the sprite on screen, in pixels
		float rate     = 1.f / 24.f;    // Seconds per frame
		p::u32 columns = 1;             // Frames per row in the sheet
		// Frames actually used by each row
		p::TArray<p::u32> numFrames{};

		p::v2_u32 currentFrame{};
		float currentFrameRemainingTime = 0.f;


		// Selects the animation to play, restarting it from its first frame
		void SetAnimation(p::u32 id);
		// Advances the animation by the time elapsed since the last call
		void NextFrame(float deltaTime);

		// Top-left corner of the current frame, in texture UV space
		p::v2 GetUV() const;
		// Bottom-right corner of the current frame, in texture UV space
		p::v2 GetUVEnd() const;

		ImTextureID GetTextureId() const
		{
			return textureId;
		}

	private:
		// Size of a single frame, in texture UV space
		p::v2 GetFrameSize() const;

		ImTextureID textureId = ImTextureID_Invalid;
	};

	// Draws the current frame of a sprite as a clickable button
	bool SpriteButton(const char* strId, const AnimatedSprite& sprite,
	    const p::LinearColor& bgColor, const p::LinearColor& tintColor);

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

	bool CollapsingHeaderWithButton(p::StringView label, ImGuiTreeNodeFlags flags,
	    bool& buttonClicked, p::StringView buttonLabel, p::v2 buttonSize = p::v2(18.f, 14.f));
}    // namespace rift::UI
