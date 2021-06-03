// Copyright 2015-2021 Piperift - All rights reserved

#include "DockSpaceLayout.h"
#include "Editors/Assets/TypePropertiesPanel.h"
#include "Editors/TypeAssetEditor.h"
#include "Lang/AST/ASTLinkage.h"

#include <Lang/Declarations/CClassDecl.h>
#include <Lang/Declarations/CFunctionDecl.h>
#include <Lang/Declarations/CStructDecl.h>
#include <Lang/Declarations/CVariableDecl.h>
#include <Lang/Identifiers/CIdentifier.h>
#include <RiftContext.h>
#include <UI/UI.h>


namespace Rift
{
	TypePropertiesPanel::TypePropertiesPanel(TypeAssetEditor& editor) : editor(editor) {}

	void TypePropertiesPanel::Draw(DockSpaceLayout& layout)
	{
		// TODO: Get AST from project or asset
		auto* ast = RiftContext::GetAST();
		if (!ast)
		{
			return;
		}

		AST::Id node = editor.GetNode();

		layout.BindNextWindowToNode(TypeAssetEditor::rightNode);

		const String name = Strings::Format(TX("Properties##{}"), editor.GetWindowId());
		if (UI::Begin(name.c_str()))
		{
			if (ast->HasAny<CClassDecl, CStructDecl>(node))    // IsStruct || IsClass
			{
				DrawVariables(*ast, node);
			}

			if (ast->HasAny<CClassDecl>(node))    // IsClass || IsFunctionLibrary
			{
				DrawFunctions(*ast, node);
			}
		}
		UI::End();
	}

	void TypePropertiesPanel::DrawVariables(AST::Tree& ast, AST::Id node)
	{
		auto* children = AST::GetLinked(ast, node);
		if (!Ensure(children))
		{
			return;
		}

		auto variableView = ast.MakeView<CVariableDecl>();
		if (UI::CollapsingHeader("Variables"))
		{
			UI::Indent(10.f);
			UI::PushStyleVar(ImGuiStyleVar_CellPadding, {1.f, 3.f});
			if (UI::BeginTable(
			        "##variableTable", 3, ImGuiTableFlags_SizingFixedFit))
			{
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
				for (AST::Id child : *children)
				{
					if (variableView.Has(child))
					{
						UI::TableNextRow();
						DrawVariable(ast, child);
					}
				}
				UI::EndTable();
			}
			UI::PopStyleVar();

			Style::PushStyleCompact();
			if (UI::Button("Add##Variable", ImVec2(-FLT_MIN, 0.0f)))
			{
				AST::Id newVariable = AST::CreateVariable(ast, "NewVariable");
				AST::Link(ast, node, newVariable);
			}
			Style::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}
	void TypePropertiesPanel::DrawFunctions(AST::Tree& ast, AST::Id node)
	{
		auto* children = AST::GetLinked(ast, node);
		if (!Ensure(children))
		{
			return;
		}

		auto functionView = ast.MakeView<CFunctionDecl>();
		if (UI::CollapsingHeader("Functions", ImGuiTreeNodeFlags_AllowItemOverlap |
		                                          ImGuiTreeNodeFlags_ClipLabelForTrailingButton))
		{
			UI::Indent(10.f);
			UI::PushStyleVar(ImGuiStyleVar_CellPadding, {1.f, 3.f});
			if (UI::BeginTable("##functionTable", 1, ImGuiTableFlags_SizingStretchSame))
			{
				for (AST::Id child : *children)
				{
					if (functionView.Has(child))
					{
						UI::TableNextRow();
						DrawFunction(ast, child);
					}
				}
				UI::EndTable();
			}
			UI::PopStyleVar();

			Style::PushStyleCompact();
			if (UI::Button("Add##Function", ImVec2(-FLT_MIN, 0.0f)))
			{
				AST::Id newFunction = AST::CreateFunction(ast, "NewFunction");
				AST::Link(ast, node, newFunction);
			}
			Style::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}

	void TypeCombo()
	{
		static StringView types[]{"bool", "i8", "u8", "i32", "u32", "float"};
		static i32 index = 5;

		if (UI::BeginCombo("##type", types[index].data(), ImGuiComboFlags_NoArrowButton))
		{
			for (int i = 0; i < 6; ++i)
			{
				UI::PushID((void*) (intptr_t) i);
				const bool itemSelected = i == index;
				const char* item_text   = types[i].data();
				if (UI::Selectable(item_text, itemSelected))
				{
					// value_changed = true;
					index = i;
				}
				if (itemSelected)
				{
					UI::SetItemDefaultFocus();
				}
				UI::PopID();
			}
			UI::EndCombo();
		}
	}

	void TypePropertiesPanel::DrawVariable(AST::Tree& ast, AST::Id id)
	{
		CIdentifier* identifier = ast.GetComponentPtr<CIdentifier>(id);
		if (!identifier)
		{
			return;
		}

		UI::PushID(identifier);

		static const LinearColor color = Color(230, 69, 69);
		static constexpr Color frameBg{122, 59, 41};
		static constexpr float frameHeight = 20.f;

		{ // Custom Selectable
			UI::TableNextColumn();
			auto& style = ImGui::GetStyle();
			Style::PushHeaderColor(LinearColor::Transparent);

			ImRect bb   = UI::GetWorkRect({0.f, frameHeight}, false, v2::One());

			bool selected = selectedNode == id;
			ImGui::Selectable("##selectArea", &selected,
				ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
				ImVec2(0, frameHeight));

			if (selected)
			{
				selectedNode = id;
			}
			else if (selectedNode == id) // If not selected but WAS selected
			{
				selectedNode = AST::NoId;
			}

			LinearColor bgColor = color.Darken(0.4f); // Inactive
			if(selected)
			{
				bgColor = color;
			}
			else if (UI::IsItemHovered())
			{
				bgColor = Style::Hovered(color);
			}
			ImGui::RenderFrame(bb.Min, bb.Max, bgColor.ToColor().DWColor(), false, 4.f);

			Style::PopHeaderColor();
		}

		UI::SameLine();
		DrawRenameInput(id, identifier);


		UI::TableNextColumn();
		{
			UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
			UI::PushItemWidth(75.f);
			TypeCombo();
			UI::PopStyleVar();
		}

		UI::TableNextColumn();
		{
			UI::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{4.f, 4.f});
			UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
			static float value = 2.f;
			UI::SetNextItemWidth(Math::Max(100.f, UI::CalcItemWidth()));
			UI::InputFloat("##defaultValue", &value, 0.f, 0.f, "%.2f");
			UI::PopStyleVar(2);
		}

		UI::PopID();
	}

	void TypePropertiesPanel::DrawFunction(AST::Tree& ast, AST::Id id)
	{
		CIdentifier* identifier = ast.GetComponentPtr<CIdentifier>(id);
		if (!identifier)
		{
			return;
		}

		UI::PushID(identifier);

		static constexpr Color color{68, 135, 229};
		static constexpr Color frameBG{41, 75, 122, 138};
		static constexpr float frameHeight = 26.f;

		auto* table = UI::GetCurrentTable();
		auto& style = ImGui::GetStyle();
		ImRect rowRect;
		rowRect.Min.x = table->WorkRect.Min.x + style.CellPadding.x;
		rowRect.Min.y = table->RowPosY1 + style.CellPadding.y;
		rowRect.Max.x = table->WorkRect.Max.x - style.CellPadding.x;
		rowRect.Max.y = table->RowPosY1 + style.CellPadding.y + frameHeight;
		ImGui::RenderFrame(rowRect.Min, rowRect.Max, color.DWColor(), false, style.FrameRounding);

		UI::PushStyleColor(ImGuiCol_FrameBg, ImVec4(frameBG));
		UI::PushStyleVar(ImGuiStyleVar_CellPadding, style.CellPadding + ImVec2{0.f, 2.f});

		// Name
		UI::TableNextColumn();
		{
			DrawRenameInput(id, identifier);
		}

		UI::PopStyleVar();
		UI::PopStyleColor();

		UI::PopID();
	}

	void TypePropertiesPanel::DrawRenameInput(AST::Id id, CIdentifier* identifier)
	{
		const bool isEditing = renameNode == id;
		if (!isEditing)
		{
			const String& name = identifier->name.ToString();

			if (UI::IsItemHovered() && UI::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				renameNode   = id;
				renameBuffer = name;
				// UI::SetFocusID(UI::GetID("##renameEdit"), UI::GetCurrentWindow());
				UI::ActivateItem(UI::GetID("##renameEdit"));
			}

			UI::PushStyleVar(ImGuiStyleVar_CellPadding, {16.f, 4.f});
			UI::PushStyleColor(ImGuiCol_FrameBg, ImVec4(Color::Transparent));

			UI::SetNextItemWidth(-FLT_MIN);
			UI::AlignTextToFramePadding();
			UI::Text(name.c_str());

			UI::PopStyleColor();
			UI::PopStyleVar();
		}
		else
		{
			UI::SetNextItemWidth(-FLT_MIN);
			UI::InputText("##renameEdit", renameBuffer, ImGuiInputTextFlags_AutoSelectAll);

			if (UI::IsItemDeactivatedAfterEdit())
			{
				identifier->name = Name{renameBuffer};
				ResetRename();
			}
			else if (UI::IsItemDeactivated())
			{
				ResetRename();
			}
		}
	}

	void TypePropertiesPanel::ResetRename()
	{
		renameNode   = AST::NoId;
		renameBuffer = {};
	}
}    // namespace Rift
