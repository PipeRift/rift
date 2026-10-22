// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "UI/Widgets.h"

#include "UI/Texture.h"

#include <IconsFontAwesome5.h>
#include <PipeTime.h>


namespace rift::UI
{
	AnimatedSprite::~AnimatedSprite()
	{
		Unload();
	}

	AnimatedSprite::AnimatedSprite(AnimatedSprite&& other) noexcept
	    : size{other.size}
	    , rate{other.rate}
	    , columns{other.columns}
	    , numFrames{p::Move(other.numFrames)}
	    , currentFrame{other.currentFrame}
	    , currentFrameRemainingTime{other.currentFrameRemainingTime}
	    , textureId{other.textureId}
	{
		other.textureId = ImTextureID_Invalid;
	}

	AnimatedSprite& AnimatedSprite::operator=(AnimatedSprite&& other) noexcept
	{
		if (this != &other)
		{
			Unload();
			size                      = other.size;
			rate                      = other.rate;
			columns                   = other.columns;
			numFrames                 = p::Move(other.numFrames);
			currentFrame              = other.currentFrame;
			currentFrameRemainingTime = other.currentFrameRemainingTime;
			textureId                 = other.textureId;

			other.textureId = ImTextureID_Invalid;
		}
		return *this;
	}

	bool AnimatedSprite::Load(p::StringView path, p::u32 framesPerRow, p::u32 animations)
	{
		Unload();

		const ImTextureID newTextureId = LoadTexture(path);
		if (newTextureId == ImTextureID_Invalid)
		{
			return false;
		}

		textureId = newTextureId;
		columns   = p::Max(framesPerRow, 1u);
		numFrames.Clear();
		numFrames.Reserve(animations);
		for (p::u32 i = 0; i < animations; ++i)
		{
			numFrames.Add(columns);
		}

		SetAnimation(0);
		return true;
	}

	void AnimatedSprite::Unload()
	{
		UnloadTexture(textureId);
		textureId = ImTextureID_Invalid;

		columns = 1;
		numFrames.Clear();
		currentFrame              = {0, 0};
		currentFrameRemainingTime = 0.f;
	}

	void AnimatedSprite::SetAnimation(p::u32 id)
	{
		currentFrame.y = numFrames.IsEmpty() ? 0 : p::Min(id, p::u32(numFrames.Size() - 1));
		currentFrame.x = 0;
		currentFrameRemainingTime = rate;
	}

	void AnimatedSprite::NextFrame(float deltaTime)
	{
		if (numFrames.IsEmpty() || rate <= 0.f)
		{
			return;
		}

		const p::u32 frames =
		    p::Max(numFrames[p::Min(currentFrame.y, p::u32(numFrames.Size() - 1))], 1u);

		currentFrameRemainingTime -= deltaTime;
		if (currentFrameRemainingTime <= 0.f)
		{
			// Jump straight to the frame the elapsed time lands on
			const p::u32 advance = p::u32(p::Floor(-currentFrameRemainingTime / rate)) + 1;
			currentFrame.x = (currentFrame.x + advance) % frames;
			currentFrameRemainingTime += float(advance) * rate;
		}
	}

	p::v2 AnimatedSprite::GetFrameSize() const
	{
		const p::u32 rows = numFrames.IsEmpty() ? 1 : p::u32(numFrames.Size());
		return p::v2{1.f / static_cast<float>(p::Max(columns, 1u)), 1.f / static_cast<float>(rows)};
	}

	p::v2 AnimatedSprite::GetUV() const
	{
		return GetFrameSize() * p::v2{float(currentFrame.x), float(currentFrame.y)};
	}

	p::v2 AnimatedSprite::GetUVEnd() const
	{
		return GetUV() + GetFrameSize();
	}

	bool SpriteButton(const char* strId, const AnimatedSprite& sprite,
	    const p::LinearColor& bgColor, const p::LinearColor& tintColor)
	{
		return UI::ImageButton(strId, ImTextureRef(sprite.GetTextureId()), sprite.size,
		    sprite.GetUV(), sprite.GetUVEnd(), bgColor, tintColor);
	}


	ImRect GetWorkRect(p::v2 desiredSize, bool addhalfItemSpacing, p::v2 extent)
	{
		auto& style  = ImGui::GetStyle();
		auto* window = UI::GetCurrentWindow();
		ImVec2 pos   = window->DC.CursorPos;
		pos.y += window->DC.CurrLineTextBaseOffset;

		const float minX = window->ParentWorkRect.Min.x;
		const float maxX = window->ParentWorkRect.Max.x;

		const ImVec2 size{p::Max(desiredSize.x, maxX - minX), desiredSize.y};
		ImRect bb{minX, pos.y, minX + size.x, pos.y + size.y};

		if (addhalfItemSpacing)
		{
			const float spacingX = 0;
			const float spacingY = style.ItemSpacing.y;
			const float spacingL = IM_TRUNC(spacingX * 0.50f);
			const float spacingU = IM_TRUNC(spacingY * 0.50f);
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

	bool MutableText(p::StringView label, p::String& text, ImGuiInputTextFlags flags)
	{
		const ImGuiID id     = UI::GetID(label);
		const bool isEditing = UI::GetActiveID() == id;
		if (!isEditing)    // Blend the input with the background while not being edited
		{
			UI::PushStyleColor(ImGuiCol_FrameBg, p::LinearColor::Transparent());
		}

		const bool valueChanged = UI::InputText(label.data(), text, flags);
		if (!isEditing)
		{
			UI::PopStyleColor();
		}
		return valueChanged;
	}

	void HelpTooltip(p::StringView text, float delay)
	{
		static ImGuiID currentHelpItemId = 0;

		ImGuiID itemId = ImGui::GetCurrentContext()->LastItemData.ID;
		if (ImGui::IsItemHovered())
		{
			bool show = true;
			if (delay > 0.f)
			{
				static p::DateTime hoverStartTime;
				const p::DateTime now = p::DateTime::Now();
				if (itemId != currentHelpItemId)
				{
					// Reset help tooltip countdown
					currentHelpItemId = itemId;
					hoverStartTime    = now;
				}
				show = (now - hoverStartTime).GetTotalSeconds() > delay;
			}

			if (show)
			{
				UI::PushStyleVar(ImGuiStyleVar_WindowPadding, p::v2{4.f, 3.f});
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				static p::String finalText;
				finalText.clear();
				p::FormatTo(finalText, "{} {}", ICON_FA_QUESTION_CIRCLE, text);
				UI::AlignTextToFramePadding();
				ImGui::TextUnformatted(finalText.c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
				UI::PopStyleVar();
			}
		}
		else if (itemId == currentHelpItemId)
		{
			currentHelpItemId = 0;
		}
	}
	void HelpMarker(p::StringView text)
	{
		ImGui::TextDisabled(ICON_FA_QUESTION_CIRCLE);
		HelpTooltip(text, 0.f);
	}

	bool CollapsingHeaderWithButton(p::StringView label, ImGuiTreeNodeFlags flags,
	    bool& buttonClicked, p::StringView buttonLabel, p::v2 buttonSize)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
		{
			return false;
		}

		ImGuiID id = window->GetID(label.data());
		flags |= ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_AllowOverlap
		       | ImGuiTreeNodeFlags_ClipLabelForTrailingButton;
		bool isOpen = ImGui::TreeNodeBehavior(id, flags, label.data());


		// Create a small overlapping close button
		// FIXME: We can evolve this into user scopeible helpers to add extra buttons on title
		// bars, headers, etc.
		// FIXME: CloseButton can overlap into text, need find a way to clip the text somehow.
		ImGuiContext& g                  = *GImGui;
		ImGuiLastItemData lastItemBackup = g.LastItemData;
		UI::PushID(id);
		const float widthAvailable =
		    ImGui::GetContentRegionAvail().x + UI::GetCurrentWindow()->DC.Indent.x;
		ImGui::SameLine(widthAvailable - 25.f);
		UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
		UI::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
		float backupPaddingY   = g.Style.FramePadding.y;
		g.Style.FramePadding.y = 0.0f;
		if (ImGui::ButtonEx(buttonLabel.data(), buttonSize, ImGuiButtonFlags_AlignTextBaseLine))
		{
			buttonClicked = true;
		}
		g.Style.FramePadding.y = backupPaddingY;
		UI::PopStyleVar(2);
		UI::PopID();
		g.LastItemData = lastItemBackup;

		return isOpen;
	}

	void DrawChipText(ImDrawList& drawList, float& x, float centerY, p::StringView text)
	{
		const ImVec2 size = UI::CalcTextSize(text);
		drawList.AddText(ImVec2{x, centerY - size.y * 0.5f}, UI::GetColorU32(ImGuiCol_Text),
		    text.data(), text.data() + text.size());
		x += size.x;
	}

	void DrawChip(ImDrawList& drawList, float& x, float centerY, p::StringView name,
	    const p::Color& color, const char* icon)
	{
		ImFont* font          = UI::GetFont();
		const float fontSize  = UI::GetFontSize();
		const float iconSize  = fontSize * 0.75f;
		const float iconWidth = icon ? font->CalcTextSizeA(iconSize, FLT_MAX, 0.f, icon).x : 0.f;

		const ImVec2 textSize    = UI::CalcTextSize(name);
		const float contentWidth = iconWidth + textSize.x;
		const float height       = UI::GetTextLineHeight() + 2.f;
		const ImVec2 chipMin{x, centerY - height * 0.5f};
		const ImVec2 chipMax{x + contentWidth + chipPadding * 2.f, centerY + height * 0.5f};

		drawList.AddRectFilled(chipMin, chipMax, color.ToPackedABGR(), chipRounding);
		// The icon shares the top of the text line, instead of being centred on its own
		if (icon)
		{
			const float textY = centerY - (iconSize * 0.5f) + 2.f;
			drawList.AddText(font, iconSize, ImVec2{chipMin.x + chipPadding, textY},
			    UI::GetColorU32(ImGuiCol_Text), icon);
		}
		const float textY = centerY - textSize.y * 0.5f;
		drawList.AddText(ImVec2{chipMin.x + chipPadding + iconWidth, textY},
		    UI::GetColorU32(ImGuiCol_Text), name.data(), name.data() + name.size());
		x = chipMax.x;
	}

	bool SelectableRow(p::StringView label, ImVec2& outMin, ImVec2& outMax,
	    const p::LinearColor& background, float height, bool highlighted)
	{
		UI::PushStyleVar(ImGuiStyleVar_ItemSpacing, p::v2{UI::GetStyle().ItemSpacing.x, 2.f});
		UI::PushStyleVar(ImGuiStyleVar_SelectableRounding, chipRounding);
		ImGuiSelectableFlags flags = ImGuiSelectableFlags_NoPadWithHalfSpacing;
		if (highlighted)
		{
			flags |= ImGuiSelectableFlags_Highlight;    // Draw as if hovered
		}
		const bool hasBackground = background.a > 0.f;
		if (hasBackground)
		{
			UI::PushHeaderColor(background);
			if (highlighted)
			{
				// A highlighted row draws a clearly brighter version of its color
				UI::PushStyleColor(ImGuiCol_HeaderHovered, background.Tint(0.2f));
			}
		}
		const bool pressed = UI::Selectable(label.data(), hasBackground, flags, p::v2{0.f, height});
		if (hasBackground)
		{
			if (highlighted)
			{
				UI::PopStyleColor();
			}
			UI::PopHeaderColor();
		}
		UI::PopStyleVar(2);

		outMin = UI::GetItemRectMin();
		outMax = UI::GetItemRectMax();
		return pressed;
	}
}    // namespace rift::UI
