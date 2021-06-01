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


	bool InputText(const char* label, String* str, ImGuiInputTextFlags flags,
	    ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_StringUserData cbUserData;
		cbUserData.str                   = str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputText(label, (char*) str->c_str(), str->capacity() + 1, flags,
		    InputTextCallback, &cbUserData);
	}

	bool InputTextMultiline(const char* label, String* str, const ImVec2& size,
	    ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_StringUserData cbUserData;
		cbUserData.str                   = str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputTextMultiline(label, (char*) str->c_str(), str->capacity() + 1, size,
		    flags, InputTextCallback, &cbUserData);
	}

	bool InputTextWithHint(const char* label, const char* hint, String* str,
	    ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_StringUserData cbUserData;
		cbUserData.str                   = str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputTextWithHint(label, hint, (char*) str->c_str(), str->capacity() + 1,
		    flags, InputTextCallback, &cbUserData);
	}
}    // namespace Rift::UI
