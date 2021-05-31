// Copyright 2015-2021 Piperift - All rights reserved

#include "DockSpaceLayout.h"
#include "Editors/Assets/TypePropertiesPanel.h"
#include "Editors/TypeAssetEditor.h"

#include <Lang/Declarations/CClassDecl.h>
#include <Lang/Declarations/CStructDecl.h>
#include <RiftContext.h>
#include <UI/UI.h>


namespace Rift
{
	TypePropertiesPanel::TypePropertiesPanel(TypeAssetEditor& editor) : editor(editor) {}

	void TypePropertiesPanel::Draw(DockSpaceLayout& layout)
	{
		const auto* ast = RiftContext::GetAST();
		AST::Id node    = editor.GetNode();

		layout.BindNextWindowToNode(TypeAssetEditor::rightNode);

		const String name = Strings::Format(TX("Properties##{}"), editor.GetWindowId());
		if (ImGui::Begin(name.c_str()))
		{
			if (ast->HasAny<CClassDecl, CStructDecl>(node))    // IsStruct || IsClass
			{
				DrawVariables();
			}

			if (ast->HasAny<CClassDecl>(node))    // IsClass || IsFunctionLibrary
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
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {1.f, 1.f});
			if (ImGui::BeginTable("##variable", 3, ImGuiTableFlags_SizingStretchSame))
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

	void TypePropertiesPanel::DrawVariable(StringView)
	{
		static FixedString<50> name{"alive"};
		static constexpr Color color{230, 69, 69};
		static constexpr Color frameBG{122, 59, 41, 138};
		static constexpr float frameHeight = 20.f;

		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(frameBG));

		auto* window = ImGui::GetCurrentWindow();
		ImRect frame;
		auto cellPadding = ImGui::GetStyle().CellPadding;
		frame.Min.x      = window->DC.CursorPos.x;
		frame.Min.y      = window->DC.CursorPos.y;
		frame.Max.x      = window->WorkRect.Max.x + cellPadding.x * 2;
		frame.Max.y      = window->DC.CursorPos.y + frameHeight + cellPadding.y * 2;
		// Framed header expand a little outside the default padding, to the edge of InnerClipRect
		frame.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
		frame.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);
		window->DrawList->AddRectFilled(frame.Min, frame.Max, color.DWColor());


		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		TypeCombo();

		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		static bool editing   = false;
		const bool wasEditing = editing;
		if (!wasEditing)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {16.f, 4.f});
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(Color::Transparent));
		}
		ImGui::InputText("##name", name.data(), name.size());
		editing = ImGui::IsItemActive();
		if (!wasEditing)
		{
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}

		ImGui::TableNextColumn();

		static float value = 2.f;
		ImGui::PushItemWidth(-FLT_MIN);
		ImGui::InputFloat("##defaultValue", &value, 0.f, 0.f, "%.1f");

		ImGui::PopStyleColor();
	}
}    // namespace Rift
