// Copyright 2015-2020 Piperift - All rights reserved

#include "Utils/Properties.h"

#include "AST/Types.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "imgui.h"
#include "Utils/Widgets.h"

#include <AST/Components/CClassDecl.h>
#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CFunctionLibraryDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CStructDecl.h>
#include <AST/Components/CVariableDecl.h>
#include <AST/Utils/FunctionUtils.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/TypeUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <Misc/EnumFlags.h>
#include <UI/UI.h>



namespace Rift
{
	using namespace EnumOperators;


	void DrawField(AST::Tree& ast, CTypeEditor& editor, AST::Id fieldId, DrawFieldFlags flags)
	{
		CIdentifier* identifier = ast.TryGet<CIdentifier>(fieldId);
		if (!identifier)
		{
			return;
		}

		UI::BeginGroup();
		UI::PushID(AST::GetIndex(fieldId));
		{
			static const Color color{230, 69, 69};
			static constexpr Color frameBg{122, 59, 41};
			static constexpr float frameHeight = 20.f;


			/*{    // Custom Selectable
			    auto& style = ImGui::GetStyle();
			    Style::PushHeaderColor(LinearColor::Transparent());

			    // ImRect bb = UI::GetWorkRect({0.f, frameHeight}, false, v2::One());
			    ImGui::Selectable("##selectArea", nullptr,
			        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap,
			        ImVec2(0, frameHeight));


			    Color bgColor = color.Shade(0.5f);
			    // UI::RenderFrame(bb.Min, bb.Max, bgColor.DWColor(), false, 2.f);

			    Style::PopHeaderColor();
			}*/

			String name = identifier->name.ToString();
			if (UI::MutableText("##name", name))
			{
				identifier->name = Name{name};
			}


			UI::SameLine();
			{
				UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
				UI::SetNextItemWidth(-FLT_MIN);
				static AST::Id selected = AST::NoId;
				Editor::TypeCombo(ast, "##type", selected);
				UI::PopStyleVar();
			}

			UI::SameLine();
			if (!HasFlag(flags, DrawFieldFlags::HideValue))
			{
				UI::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{4.f, 4.f});
				UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
				static float value = 2.f;
				UI::SetNextItemWidth(-FLT_MIN);
				UI::InputFloat("##defaultValue", &value, 0.f, 0.f, "%.2f");
				UI::PopStyleVar(2);
			}
		}
		UI::PopID();
		UI::EndGroup();
		if (UI::IsItemHovered() && UI::IsKeyReleased(GLFW_KEY_DELETE))
		{
			editor.pendingDeletePropertyId = fieldId;
		}
		if (UI::BeginPopupContextItem("##FieldContextMenu"))
		{
			if (UI::MenuItem("Delete"))
			{
				editor.pendingDeletePropertyId = fieldId;
			}
			UI::EndPopup();
		}
	}

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
			UI::RenderFrame(bb.Min, bb.Max, bgColor.DWColor(), false, 2.f);

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

		if (auto* children = AST::Hierarchy::GetChildren(ast, functionId))
		{
			for (AST::Id childId : *children)
			{
				DrawField(ast, editor, childId);
			}
		}
		if (UI::Button(ICON_FA_PLUS "##FunctionInput"))
		{
			AST::Functions::AddInputArgument(ast, functionId);
		}

		UI::PopID();
	}

	void DrawVariables(AST::Tree& ast, CTypeEditor& editor, AST::Id typeId)
	{
		if (UI::CollapsingHeader("Variables", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto variableView = ast.Filter<CVariableDecl>();
			UI::Indent(10.f);
			if (auto* children = AST::Hierarchy::GetChildren(ast, typeId))
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
			if (UI::Button(ICON_FA_PLUS "##Variable", ImVec2(-FLT_MIN, 0.0f)))
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
			auto functionView = ast.Filter<CFunctionDecl>();
			UI::Indent(10.f);

			if (auto* children = AST::Hierarchy::GetChildren(ast, typeId))
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
			if (UI::Button(ICON_FA_PLUS "##Function", ImVec2(-FLT_MIN, 0.0f)))
			{
				Types::AddFunction({ast, typeId}, "NewFunction");
			}
			Style::PopStyleCompact();
			UI::Unindent(10.f);
			UI::Dummy(ImVec2(0.0f, 10.0f));
		}
	}

	void DrawProperties(AST::Tree& ast, AST::Id typeId)
	{
		auto& editor = ast.Get<CTypeEditor>(typeId);

		const String windowName = Strings::Format("Properties##{}", typeId);
		if (UI::Begin(windowName.c_str()))
		{
			if (Types::CanContainVariables(ast, typeId))
			{
				DrawVariables(ast, editor, typeId);
			}

			if (Types::CanContainFunctions(ast, typeId))
			{
				DrawFunctions(ast, editor, typeId);
			}
		}
		UI::End();

		if (!IsNone(editor.pendingDeletePropertyId))
		{
			AST::Hierarchy::RemoveDeep(ast, editor.pendingDeletePropertyId);
			editor.pendingDeletePropertyId = AST::NoId;
		}
	}
}    // namespace Rift
