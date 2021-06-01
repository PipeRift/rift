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
#include <UI/Style.h>
#include <UI/UI.h>
#include <UI/Widgets.h>


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
			UI::PushStyleVar(ImGuiStyleVar_CellPadding, {1.f, 3.f});
			if (UI::BeginTable("##variableTable", 3, ImGuiTableFlags_SizingStretchSame))
			{
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
			UI::Spacing();
		}
	}
	void TypePropertiesPanel::DrawFunctions(AST::Tree& ast, AST::Id node)
	{
		auto functionView = ast.MakeView<CFunctionDecl>();
		if (UI::CollapsingHeader("Functions"))
		{
			auto* children = AST::GetLinked(ast, node);
			if (!Ensure(children))
			{
				return;
			}

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
			UI::Spacing();
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

		static constexpr Color color{230, 69, 69};
		static constexpr Color frameBG{122, 59, 41, 138};
		static constexpr float frameHeight = 26.f;

		auto* table = UI::GetCurrentTable();
		auto& style = ImGui::GetStyle();
		ImRect rowRect;
		rowRect.Min.x = table->WorkRect.Min.x + style.CellPadding.x;
		rowRect.Min.y = table->RowPosY1 + style.CellPadding.y;
		rowRect.Max.x = table->WorkRect.Max.x - style.CellPadding.x;
		rowRect.Max.y = table->RowPosY1 + frameHeight - style.CellPadding.y;
		ImGui::RenderFrame(rowRect.Min, rowRect.Max, color.DWColor(), false, style.FrameRounding);

		UI::PushStyleColor(ImGuiCol_FrameBg, ImVec4(frameBG));
		UI::PushStyleVar(ImGuiStyleVar_CellPadding, style.CellPadding + ImVec2{0.f, 2.f});

		UI::TableNextColumn();
		{
			UI::PushItemWidth(-FLT_MIN);
			TypeCombo();
		}

		UI::TableNextColumn();
		{
			UI::PushItemWidth(-FLT_MIN);
			DrawRenameInput(id, identifier);
		}

		UI::TableNextColumn();
		{
			static float value = 2.f;
			UI::PushItemWidth(-FLT_MIN);
			UI::InputFloat("##defaultValue", &value, 0.f, 0.f, "%.1f");
		}

		UI::PopStyleVar();
		UI::PopStyleColor();

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
		rowRect.Max.y = table->RowPosY1 + frameHeight - style.CellPadding.y;
		ImGui::RenderFrame(rowRect.Min, rowRect.Max, color.DWColor(), false, style.FrameRounding);

		UI::PushStyleColor(ImGuiCol_FrameBg, ImVec4(frameBG));
		UI::PushStyleVar(ImGuiStyleVar_CellPadding, style.CellPadding + ImVec2{0.f, 2.f});

		// Name
		UI::TableNextColumn();
		{
			UI::PushItemWidth(-FLT_MIN);
			DrawRenameInput(id, identifier);
		}

		UI::PopStyleVar();
		UI::PopStyleColor();

		UI::PopID();
	}

	void TypePropertiesPanel::DrawRenameInput(AST::Id id, CIdentifier* identifier)
	{
		String* name;
		String currentName;

		const bool wasEditing = renameNode == id;
		if (!wasEditing)
		{
			currentName = identifier->name.ToString();
			name        = &currentName;

			UI::PushStyleVar(ImGuiStyleVar_CellPadding, {16.f, 4.f});
			UI::PushStyleColor(ImGuiCol_FrameBg, ImVec4(Color::Transparent));
		}
		else
		{
			name = &renameBuffer;
		}

		UI::InputText("##rename", name);

		if (UI::IsItemDeactivatedAfterEdit())
		{
			identifier->name = Name{renameBuffer};
			ResetRename();
		}
		else if (UI::IsItemActive())
		{
			renameNode   = id;
			renameBuffer = identifier->name.ToString();
		}
		else if (wasEditing)    // If not active and editing, stop
		{
			ResetRename();
		}

		if (!wasEditing)
		{
			UI::PopStyleColor();
			UI::PopStyleVar();
		}
	}

	void TypePropertiesPanel::ResetRename()
	{
		renameNode   = AST::NoId;
		renameBuffer = {};
	}
}    // namespace Rift
