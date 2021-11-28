// Copyright 2015-2020 Piperift - All rights reserved

#include "Utils/Properties.h"

#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"

#include <AST/Components/CClassDecl.h>
#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CFunctionLibraryDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CStructDecl.h>
#include <AST/Components/CVariableDecl.h>
#include <AST/Hierarchy.h>
#include <AST/Utils/FunctionUtils.h>
#include <AST/Utils/TypeUtils.h>
#include <GLFW/glfw3.h>
#include <UI/UI.h>


namespace Rift
{
	void DrawVariable(AST::Tree& ast, CTypeEditor& editor, AST::Id variableId)
	{
		CIdentifier* identifier = ast.TryGet<CIdentifier>(variableId);
		if (!identifier)
		{
			return;
		}

		ImGui::PushID(identifier);

		static const Color color{230, 69, 69};
		static constexpr Color frameBg{122, 59, 41};
		static constexpr float frameHeight = 20.f;

		UI::TableNextColumn();
		{    // Custom Selectable
			auto& style = ImGui::GetStyle();
			Style::PushHeaderColor(LinearColor::Transparent());

			ImRect bb = UI::GetWorkRect({0.f, frameHeight}, false, v2::One());

			bool selected = editor.selectedPropertyId == variableId;
			ImGui::Selectable("##selectArea", &selected,
			    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
			    ImVec2(0, frameHeight));

			if (selected)
			{
				editor.selectedPropertyId = variableId;

				if (UI::IsKeyReleased(GLFW_KEY_DELETE))
				{
					editor.pendingDeletePropertyId = variableId;
				}
			}
			else if (editor.selectedPropertyId == variableId)    // If not selected but WAS selected
			{
				editor.selectedPropertyId = AST::NoId;
			}

			Color bgColor = color.Shade(0.5f);
			if (selected)
			{
				bgColor = color;
			}
			else if (UI::IsItemHovered())
			{
				bgColor = Style::Hovered(color);
			}
			UI::RenderFrame(bb.Min, bb.Max, bgColor.DWColor(), false, 3.f);

			Style::PopHeaderColor();
		}

		if (UI::BeginPopupContextItem())
		{
			if (UI::MenuItem("Delete"))
			{
				editor.pendingDeletePropertyId = variableId;
			}
			UI::EndPopup();
		}

		UI::SameLine();

		String name = identifier->name.ToString();
		if (UI::MutableText("##name", name))
		{
			identifier->name = Name{name};
		}


		UI::TableNextColumn();
		{
			UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
			UI::SetNextItemWidth(-FLT_MIN);
			// TypeCombo();
			UI::PopStyleVar();
		}

		UI::TableNextColumn();
		{
			UI::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{4.f, 4.f});
			UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
			static float value = 2.f;
			UI::SetNextItemWidth(-FLT_MIN);
			UI::InputFloat("##defaultValue", &value, 0.f, 0.f, "%.2f");
			UI::PopStyleVar(2);
		}

		UI::PopID();
	}

	void DrawFunction(AST::Tree& ast, CTypeEditor& editor, AST::Id functionId)
	{
		CIdentifier* identifier = ast.TryGet<CIdentifier>(functionId);
		if (!identifier)
		{
			return;
		}

		ImGui::PushID(identifier);

		static constexpr Color color{68, 135, 229};
		static constexpr Color frameBG{41, 75, 122, 138};
		static constexpr float frameHeight = 20.f;

		UI::TableNextColumn();
		{    // Custom Selectable
			auto& style = ImGui::GetStyle();
			Style::PushHeaderColor(LinearColor::Transparent());

			ImRect bb = UI::GetWorkRect({0.f, frameHeight}, false, v2::One());

			bool selected = editor.selectedPropertyId == functionId;
			ImGui::Selectable("##selectArea", &selected,
			    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
			    ImVec2(0, frameHeight));

			if (selected)
			{
				editor.selectedPropertyId = functionId;

				if (UI::IsKeyReleased(GLFW_KEY_DELETE))
				{
					editor.pendingDeletePropertyId = functionId;
				}
			}
			else if (editor.selectedPropertyId == functionId)    // If not selected but WAS selected
			{
				editor.selectedPropertyId = AST::NoId;
			}

			Color bgColor = color.Shade(0.5f);
			if (selected)
			{
				bgColor = color;
			}
			else if (UI::IsItemHovered())
			{
				bgColor = Style::Hovered(color);
			}
			ImGui::RenderFrame(bb.Min, bb.Max, bgColor.DWColor(), false, 3.f);

			Style::PopHeaderColor();
		}

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
			{
				editor.pendingDeletePropertyId = functionId;
			}
			ImGui::EndPopup();
		}

		UI::SameLine();
		String name = identifier->name.ToString();
		if (UI::MutableText("##name", name))
		{
			identifier->name = Name{name};
		}

		UI::PopID();
	}

	void DrawVariables(AST::Tree& ast, CTypeEditor& editor, AST::Id typeId)
	{
		if (UI::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto variableView = ast.Query<CVariableDecl>();
			UI::Indent(10.f);
			if (auto* children = AST::GetLinked(ast, typeId))
			{
				UI::PushStyleVar(ImGuiStyleVar_CellPadding, {1.f, 3.f});
				if (UI::BeginTable("##variableTable", 3, ImGuiTableFlags_SizingFixedFit))
				{
					ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.45f);
					ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.25f);
					ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.30f);
					for (AST::Id child : *children)
					{
						if (variableView.Has(child))
						{
							UI::TableNextRow();
							DrawVariable(ast, editor, child);
						}
					}
					UI::EndTable();
				}
				UI::PopStyleVar();
			}

			Style::PushStyleCompact();
			if (UI::Button("Add##Variable", ImVec2(-FLT_MIN, 0.0f)))
			{
				Types::AddVariable({ast, typeId}, "NewVariable");
			}
			Style::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}

	void DrawFunctions(AST::Tree& ast, CTypeEditor& editor, AST::Id typeId)
	{
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen
		                               | ImGuiTreeNodeFlags_AllowItemOverlap
		                               | ImGuiTreeNodeFlags_ClipLabelForTrailingButton;
		if (UI::CollapsingHeader("Functions", flags))
		{
			auto functionView = ast.Query<CFunctionDecl>();
			UI::Indent(10.f);

			if (auto* children = AST::GetLinked(ast, typeId))
			{
				UI::PushStyleVar(ImGuiStyleVar_CellPadding, {1.f, 3.f});
				if (UI::BeginTable("##functionTable", 1, ImGuiTableFlags_SizingFixedFit))
				{
					ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);

					for (AST::Id child : *children)
					{
						if (functionView.Has(child))
						{
							UI::TableNextRow();
							DrawFunction(ast, editor, child);
						}
					}
					UI::EndTable();
				}
				UI::PopStyleVar();
			}

			Style::PushStyleCompact();
			if (UI::Button("Add##Function", ImVec2(-FLT_MIN, 0.0f)))
			{
				AST::Id newFunction = Types::AddFunction({ast, typeId}, "NewFunction");
				AST::AddChildren(ast, typeId, newFunction);
			}
			Style::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}

	void DrawProperties(AST::Tree& ast, AST::Id typeId, struct DockSpaceLayout& layout)
	{
		auto& editor = ast.Get<CTypeEditor>(typeId);

		layout.BindNextWindowToNode(CTypeEditor::rightNode);
		const String windowName = Strings::Format("Properties##{}", typeId);
		if (UI::Begin(windowName.c_str()))
		{
			// IsStruct || IsClass
			if (ast.HasAny<CClassDecl, CStructDecl>(typeId))
			{
				DrawVariables(ast, editor, typeId);
			}

			// IsClass || IsFunctionLibrary
			if (ast.HasAny<CClassDecl, CFunctionLibraryDecl>(typeId))
			{
				DrawFunctions(ast, editor, typeId);
			}
		}
		UI::End();

		if (!IsNone(editor.pendingDeletePropertyId))
		{
			AST::RemoveDeep(ast, editor.pendingDeletePropertyId);
			editor.pendingDeletePropertyId = AST::NoId;
		}
	}
}    // namespace Rift
