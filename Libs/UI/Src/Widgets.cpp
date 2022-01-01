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
		return UI::ImageButton(
		    sprite.textureId, sprite.size, uv, uv + sprite.size, framePadding, bgColor, tintColor);
	}


	struct InputTextCallbackStringUserData
	{
		String* str;
		ImGuiInputTextCallback chainCallback;
		void* chainCallbackUserData;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		auto* userData = static_cast<InputTextCallbackStringUserData*>(data->UserData);
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we
			// need to set them back to what we want.
			String* str = userData->str;
			IM_ASSERT(data->Buf == str->c_str());
			str->resize(data->BufTextLen);
			data->Buf = (char*)str->c_str();
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

		InputTextCallbackStringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputText(
		    label, (char*)str.c_str(), str.capacity() + 1, flags, InputTextCallback, &cbUserData);
	}

	bool InputTextMultiline(const char* label, String& str, const ImVec2& size,
	    ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallbackStringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputTextMultiline(label, (char*)str.c_str(), str.capacity() + 1, size, flags,
		    InputTextCallback, &cbUserData);
	}

	bool InputTextWithHint(const char* label, const char* hint, String& str,
	    ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallbackStringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputTextWithHint(label, hint, (char*)str.c_str(), str.capacity() + 1, flags,
		    InputTextCallback, &cbUserData);
	}

	ImRect GetWorkRect(v2 desiredSize, bool addhalfItemSpacing, v2 extent)
	{
		auto& style  = ImGui::GetStyle();
		auto* table  = UI::GetCurrentTable();
		auto* window = UI::GetCurrentWindow();
		ImVec2 pos   = window->DC.CursorPos;
		pos.y += window->DC.CurrLineTextBaseOffset;

		const float minX = window->ParentWorkRect.Min.x;
		const float maxX = window->ParentWorkRect.Max.x;

		const ImVec2 size{Math::Max(desiredSize.x, maxX - minX), desiredSize.y};
		ImRect bb{minX, pos.y, minX + size.x, pos.y + size.y};

		if (addhalfItemSpacing)
		{
			const float spacingX = 0;
			const float spacingY = style.ItemSpacing.y;
			const float spacingL = IM_FLOOR(spacingX * 0.50f);
			const float spacingU = IM_FLOOR(spacingY * 0.50f);
			bb.Min.x -= spacingL;
			bb.Min.y -= spacingU;
			bb.Max.x += (spacingX - spacingL);
			bb.Max.y += (spacingY - spacingU);
		}
		bb.Min.x -= extent.x;
		bb.Min.y -= extent.y;
		bb.Max.x += extent.x;
		bb.Max.y += extent.y;
		return bb;
	}

	static ImGuiID gPendingEditingId = 0;

	bool MutableText(StringView label, String& text, ImGuiInputTextFlags flags)
	{
		const ImGuiID id     = UI::GetID(label);
		const bool isEditing = UI::GetActiveID() == id;
		if (!isEditing)    // Is editing
		{
			UI::PushStyleColor(ImGuiCol_FrameBg, LinearColor::Transparent());
		}
		else
		{}

		const bool valueChanged = UI::InputText(label.data(), text, flags);
		if (!isEditing)
		{
			UI::PopStyleColor();
		}
		return valueChanged;
	}
}    // namespace Rift::UI
