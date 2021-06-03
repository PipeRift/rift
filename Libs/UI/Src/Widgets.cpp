// Copyright 2015-2021 Piperift - All rights reserved

#include "UI/Widgets.h"


namespace Rift::UI
{
	void AnimatedSprite::SetAnimation(u32 id)
	{
		currentFrame              = {0, Math::Clamp(id, 0u, u32(numFrames.Size() - 1))};
		currentFrameRemainingTime = rate;
	}

	void AnimatedSprite::NextFrame(float deltaTime)
	{
		currentFrameRemainingTime -= deltaTime;

		if (currentFrameRemainingTime <= 0.f)
		{
			++currentFrame.x;
			if (currentFrame.x >= numFrames[currentFrame.y])
			{
				currentFrame.x = 0;
			}
			currentFrameRemainingTime = rate;
		}
	}

	v2 AnimatedSprite::GetUV() const
	{
		return size * currentFrame;
	}


	bool SpriteButton(AnimatedSprite& sprite, i32 framePadding, const LinearColor& bgColor,
	    const LinearColor& tintColor)
	{
		const v2 uv = sprite.GetUV();
		return UI::ImageButton(sprite.textureId, sprite.size, uv, v2(uv + sprite.size),
		    framePadding, bgColor, tintColor);
	}


	struct InputTextCallback_StringUserData
	{
		String* str;
		ImGuiInputTextCallback chainCallback;
		void* chainCallbackUserData;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		auto* userData = static_cast<InputTextCallback_StringUserData*>(data->UserData);
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we
			// need to set them back to what we want.
			String* str = userData->str;
			IM_ASSERT(data->Buf == str->c_str());
			str->resize(data->BufTextLen);
			data->Buf = (char*) str->c_str();
		}
		else if (userData->chainCallback)
		{
			// Forward to user callback, if any
			data->UserData = userData->chainCallbackUserData;
			return userData->chainCallback(data);
		}
		return 0;
	}


	bool InputText(const char* label, String& str, ImGuiInputTextFlags flags,
	    ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_StringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputText(
		    label, (char*) str.c_str(), str.capacity() + 1, flags, InputTextCallback, &cbUserData);
	}

	bool InputTextMultiline(const char* label, String& str, const ImVec2& size,
	    ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_StringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputTextMultiline(label, (char*) str.c_str(), str.capacity() + 1, size,
		    flags, InputTextCallback, &cbUserData);
	}

	bool InputTextWithHint(const char* label, const char* hint, String& str,
	    ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_StringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputTextWithHint(label, hint, (char*) str.c_str(), str.capacity() + 1, flags,
		    InputTextCallback, &cbUserData);
	}

	ImRect GetWorkRect(v2 desiredSize, bool addhalfItemSpacing, v2 extent)
	{
		auto& style  = ImGui::GetStyle();
		auto* table  = UI::GetCurrentTable();
		auto* window = UI::GetCurrentWindow();
		ImVec2 pos   = window->DC.CursorPos;
		pos.y += window->DC.CurrLineTextBaseOffset;

		const float min_x = window->ParentWorkRect.Min.x;
		const float max_x = window->ParentWorkRect.Max.x;

		const ImVec2 size{Math::Max(desiredSize.x, max_x - min_x), desiredSize.y};
		ImRect bb{min_x, pos.y, min_x + size.x, pos.y + size.y};

		if (addhalfItemSpacing)
		{
			const float spacing_x = 0;
			const float spacing_y = style.ItemSpacing.y;
			const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
			const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
			bb.Min.x -= spacing_L;
			bb.Min.y -= spacing_U;
			bb.Max.x += (spacing_x - spacing_L);
			bb.Max.y += (spacing_y - spacing_U);
		}
		bb.Min.x -= extent.x;
		bb.Min.y -= extent.y;
		bb.Max.x += extent.x;
		bb.Max.y += extent.y;
		return bb;
	}
}    // namespace Rift::UI
