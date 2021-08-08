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
#include <AST/Linkage.h>
#include <GLFW/glfw3.h>
#include <UI/UI.h>


namespace Rift
{
	void DrawVariable(AST::Tree& ast, AST::Id variableId)
	{
		CIdentifier* identifier = ast.TryGet<CIdentifier>(variableId);
		if (!identifier)
		{
			return;
		}

		ImGui::PushID(identifier);

		static const LinearColor color = Color(230, 69, 69);
		static constexpr Color frameBg{122, 59, 41};
		static constexpr float frameHeight = 20.f;

		UI::TableNextColumn();
		{    // Custom Selectable
			auto& style = ImGui::GetStyle();
			Style::PushHeaderColor(LinearColor::Transparent);

			ImRect bb = UI::GetWorkRect({0.f, frameHeight}, false, v2::One());

			bool selected = false;    // selectedNode == variableId;
			ImGui::Selectable("##selectArea", &selected,
			    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
			    ImVec2(0, frameHeight));

			if (selected)
			{
				// selectedNode = variableId;

				if (UI::IsKeyReleased(GLFW_KEY_DELETE))
				{
					// pendingDelete = variableId;
				}
			}
			else if (false /*selectedNode == variableId*/)    // If not selected but WAS selected
			{
				// selectedNode = AST::NoId;
			}

			LinearColor bgColor = color.Darken(0.5f);
			if (selected)
			{
				bgColor = color;
			}
			else if (UI::IsItemHovered())
			{
				bgColor = Style::Hovered(color);
			}
			UI::RenderFrame(bb.Min, bb.Max, bgColor.ToColor().DWColor(), false, 3.f);

			Style::PopHeaderColor();
		}

		if (UI::BeginPopupContextItem())
		{
			if (UI::MenuItem("Delete"))
			{
				// pendingDelete = variableId;
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

	void DrawFunction(AST::Tree& ast, AST::Id functionId)
	{
		CIdentifier* identifier = ast.TryGet<CIdentifier>(functionId);
		if (!identifier)
		{
			return;
		}

		ImGui::PushID(identifier);

		static LinearColor color = Color{68, 135, 229};
		static constexpr Color frameBG{41, 75, 122, 138};
		static constexpr float frameHeight = 20.f;

		UI::TableNextColumn();
		{    // Custom Selectable
			auto& style = ImGui::GetStyle();
			Style::PushHeaderColor(LinearColor::Transparent);

			ImRect bb = UI::GetWorkRect({0.f, frameHeight}, false, v2::One());

			bool selected = false;    // selectedNode == functionId;
			ImGui::Selectable("##selectArea", &selected,
			    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
			    ImVec2(0, frameHeight));

			if (selected)
			{
				// selectedNode = functionId;

				if (UI::IsKeyReleased(GLFW_KEY_DELETE))
				{
					// pendingDelete = functionId;
				}
			}
			else if (false /*selectedNode == functionId*/)    // If not selected but WAS selected
			{
				// selectedNode = AST::NoId;
			}

			LinearColor bgColor = color.Darken(0.5f);
			if (selected)
			{
				bgColor = color;
			}
			else if (UI::IsItemHovered())
			{
				bgColor = Style::Hovered(color);
			}
			ImGui::RenderFrame(bb.Min, bb.Max, bgColor.ToColor().DWColor(), false, 3.f);

			Style::PopHeaderColor();
		}

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete"))
			{
				// pendingDelete = functionId;
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

	void DrawVariables(AST::Tree& ast, AST::Id typeId)
	{
		if (UI::CollapsingHeader("Variables"))
		{
			auto variableView = ast.MakeView<CVariableDecl>();
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
							DrawVariable(ast, child);
						}
					}
					UI::EndTable();
				}
				UI::PopStyleVar();
			}

			Style::PushStyleCompact();
			if (UI::Button("Add##Variable", ImVec2(-FLT_MIN, 0.0f)))
			{
				AST::Id newVariable = AST::CreateVariable(ast, "NewVariable");
				AST::Link(ast, typeId, newVariable);
			}
			Style::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}

	void DrawFunctions(AST::Tree& ast, AST::Id typeId)
	{
		if (UI::CollapsingHeader("Functions", ImGuiTreeNodeFlags_AllowItemOverlap |
		                                          ImGuiTreeNodeFlags_ClipLabelForTrailingButton))
		{
			auto functionView = ast.MakeView<CFunctionDecl>();
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
							DrawFunction(ast, child);
						}
					}
					UI::EndTable();
				}
				UI::PopStyleVar();
			}

			Style::PushStyleCompact();
			if (UI::Button("Add##Function", ImVec2(-FLT_MIN, 0.0f)))
			{
				AST::Id newFunction = AST::CreateFunction(ast, "NewFunction");
				AST::Link(ast, typeId, newFunction);
			}
			Style::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}

	void DrawProperties(AST::Tree& ast, AST::Id typeId, struct DockSpaceLayout& layout)
	{
		layout.BindNextWindowToNode(CTypeEditor::rightNode);

		const String windowName = Strings::Format("Properties##{}", typeId);
		if (UI::Begin(windowName.c_str()))
		{
			// IsStruct || IsClass
			if (ast.HasAny<CClassDecl, CStructDecl>(typeId))
			{
				DrawVariables(ast, typeId);
			}

			// IsClass || IsFunctionLibrary
			if (ast.HasAny<CClassDecl, CFunctionLibraryDecl>(typeId))
			{
				DrawFunctions(ast, typeId);
			}
		}
		UI::End();

		// if (!IsNone(pendingDelete))
		//{
		//	AST::RemoveDeep(ast, pendingDelete);
		//	pendingDelete = AST::NoId;
		//}
	}
}    // namespace Rift
