// Copyright 2015-2021 Piperift - All rights reserved

#include "DockSpaceLayout.h"
#include "Editors/AssetEditor.h"
#include "Editors/Assets/TypePropertiesPanel.h"

#include <UI/UI.h>


namespace Rift
{
	void TypePropertiesPanel::Draw(StringView baseId, DockSpaceLayout& layout)
	{
		layout.BindNextWindowToNode(AssetEditor::rightNode);

		const String name = Strings::Format(TX("Properties##{}"), baseId);
		if (ImGui::Begin(name.c_str()))
		{
			if (true)    // IsStruct || IsClass
			{
				DrawVariables();
			}

			if (true)    // IsClass || IsFunctionLibrary
			{
				DrawFunctions();
			}
		}
		ImGui::End();
	}

	void TypePropertiesPanel::DrawVariables()
	{
		if (ImGui::CollapsingHeader("Variables"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {0.f, 0.f});
			if (ImGui::BeginTable("##variable", 3))
			{
				ImGui::TableNextRow();
				DrawVariable("alive");
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();
		}
	}
	void TypePropertiesPanel::DrawFunctions()
	{
		if (ImGui::CollapsingHeader("Functions"))
		{
			ImGui::Button("Add");
		}
	}

	void TypeCombo()
	{
		static StringView types[]{"bool", "i8", "u8", "i32", "u32", "float"};
		static i32 index = 5;

		if (ImGui::BeginCombo("##type", types[index].data(), ImGuiComboFlags_NoArrowButton))
		{
			for (int i = 0; i < 6; ++i)
			{
				ImGui::PushID((void*) (intptr_t) i);
				const bool itemSelected = i == index;
				const char* item_text   = types[i].data();
				if (ImGui::Selectable(item_text, itemSelected))
				{
					// value_changed = true;
					index = i;
				}
				if (itemSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
	}

	void TypePropertiesPanel::DrawVariable(StringView name)
	{
		static constexpr Color color{230, 69, 69};
		static constexpr Color frameBG{122, 59, 41, 138};
		static constexpr float frameHeight = 20.f;

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(frameBG));

		auto* window = ImGui::GetCurrentWindow();
		ImRect frame;
		frame.Min.x = window->WorkRect.Min.x;    // window->DC.CursorPos.x;
		frame.Min.y = window->DC.CursorPos.y;
		frame.Max.x = window->WorkRect.Max.x;
		frame.Max.y = window->DC.CursorPos.y + frameHeight;

		// Framed header expand a little outside the default padding, to the edge of InnerClipRect
		// (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x
		// instead of WindowPadding.x*0.5f)
		frame.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
		frame.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);

		window->DrawList->AddRectFilled(frame.Min, frame.Max, color.DWColor());


		ImGui::TableNextColumn();
		TypeCombo();
		ImGui::TableNextColumn();

		ImGui::Text(name.data());

		ImGui::TableNextColumn();

		static float value = 2.f;
		ImGui::InputFloat("##defaultValue", &value);

		ImGui::PopStyleColor();
	}


	AST::Id TypePropertiesPanel::GetOwnerNode() const
	{
		if (!owner)
		{
			return AST::NoId;
		}

		TAssetPtr<TypeAsset> asset = owner->GetAsset();
		if (!asset.IsValid())
		{
			return AST::NoId;
		}

		return asset->declaration;
	}
}    // namespace Rift
