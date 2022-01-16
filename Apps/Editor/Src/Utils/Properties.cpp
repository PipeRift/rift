// Copyright 2015-2020 Piperift - All rights reserved

#include "Utils/Properties.h"

#include "AST/Types.h"
#include "Components/CTypeEditor.h"
#include "DockSpaceLayout.h"
#include "UI/Style.h"
#include "UI/Widgets.h"
#include "Utils/EditorColors.h"
#include "Utils/Widgets.h"

#include <AST/Components/CClassDecl.h>
#include <AST/Components/CExpressionInput.h>
#include <AST/Components/CExpressionOutputs.h>
#include <AST/Components/CFunctionDecl.h>
#include <AST/Components/CFunctionLibraryDecl.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CParameterDecl.h>
#include <AST/Components/CStructDecl.h>
#include <AST/Components/CVariableDecl.h>
#include <AST/Utils/FunctionUtils.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/TransactionUtils.h>
#include <AST/Utils/TypeUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <Misc/EnumFlags.h>
#include <UI/UI.h>


namespace Rift
{
	using namespace EnumOperators;


	void DrawField(AST::Tree& ast, CTypeEditor& editor, AST::Id functionId, AST::Id fieldId,
	    DrawFieldFlags flags)
	{
		CIdentifier* identifier = ast.TryGet<CIdentifier>(fieldId);
		auto* paramDecl         = ast.TryGet<CParameterDecl>(fieldId);
		if (!identifier || !paramDecl)
		{
			return;
		}

		UI::BeginGroup();
		UI::PushID(AST::GetIndex(fieldId));
		{
			const Color color                  = Style::GetTypeColor(ast, paramDecl->typeId);
			static constexpr float frameHeight = 20.f;
			{    // Custom Selectable
				Color bgColor = color;
				ImRect bb     = UI::GetWorkRect({0.f, frameHeight}, false, v2::One());
				UI::RenderFrame(bb.Min, bb.Max, bgColor.DWColor(), false, 2.f);
			}

			UI::SetNextItemWidth(UI::GetContentRegionAvailWidth() * 0.5f);
			static String nameId;
			nameId.clear();
			Strings::FormatTo(nameId, " ##name_{}", fieldId);
			String name = identifier->name.ToString();
			if (UI::MutableText(nameId, name,
			        ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
			{
				ScopedChange(ast, functionId);
				identifier->name = Name{name};
			}


			UI::SameLine();
			{
				UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
				UI::SetNextItemWidth(-FLT_MIN);
				Editor::TypeCombo(ast, "##type", paramDecl->typeId);
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
		static String contextName;
		contextName.clear();
		Strings::FormatTo(contextName, "##FieldContextMenu_{}", fieldId);
		if (UI::BeginPopupContextItem(contextName.c_str()))
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
		auto* variableDecl      = ast.TryGet<CVariableDecl>(variableId);
		if (!identifier || !variableDecl)
		{
			return;
		}

		ImGui::PushID(identifier);

		const Color color                  = Style::GetTypeColor(ast, variableDecl->typeId);
		static constexpr float frameHeight = 20.f;

		UI::TableNextColumn();
		{    // Custom Selectable
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

			Color bgColor = color;
			if (selected)
			{
				bgColor = color.Tint(0.1f);
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

		UI::SetNextItemWidth(-FLT_MIN);
		static String nameId;
		nameId.clear();
		Strings::FormatTo(nameId, " ##name_{}", variableId);
		String name = identifier->name.ToString();
		if (UI::MutableText(nameId, name,
		        ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			identifier->name = Name{name};
		}

		UI::TableNextColumn();
		{
			UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
			UI::SetNextItemWidth(-FLT_MIN);
			Editor::TypeCombo(ast, "##type", variableDecl->typeId);
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

		String functionName = identifier->name.ToString();

		Style::PushHeaderColor(Style::callColor);
		UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
		static String headerId;
		headerId.clear();
		Strings::FormatTo(headerId, " ###header_{}", functionId);
		bool visible = true;
		bool open    = UI::CollapsingHeader(headerId.c_str(), &visible);
		UI::PopStyleVar();
		Style::PopHeaderColor();

		if (!visible || (UI::IsItemHovered() && UI::IsKeyReleased(GLFW_KEY_DELETE)))
		{
			editor.pendingDeletePropertyId = functionId;
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
		// Expand text input to the right excluding the close button
		auto* g = UI::GetCurrentContext();
		UI::SetNextItemWidth(
		    UI::GetContentRegionAvailWidth() - (g->Style.FramePadding.x * 2.0f + g->FontSize));
		static String inputId;
		inputId.clear();
		Strings::FormatTo(inputId, "##name_{}", functionId);
		if (UI::MutableText(inputId, functionName,
		        ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			identifier->name = Name{functionName};
		}

		if (open)
		{
			static constexpr float extraIndent = 10.f;
			UI::Indent(extraIndent);

			if (UI::BeginTable("##fieldsTable", 2, ImGuiTableFlags_SizingFixedFit))
			{
				UI::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.5f);
				UI::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.5f);

				UI::TableNextRow();
				UI::TableNextColumn();
				if (auto* children = AST::Hierarchy::GetChildren(ast, functionId))
				{
					auto exprOutputs = ast.Filter<CExpressionOutputs>();
					for (AST::Id childId : *children)
					{
						if (exprOutputs.Has(childId))
						{
							DrawField(ast, editor, functionId, childId);
						}
					}
				}
				Style::PushStyleCompact();
				if (UI::Button(ICON_FA_PLUS "##FunctionInput", ImVec2(-FLT_MIN, 0.0f)))
				{
					ScopedChange(ast, functionId);
					AST::Functions::AddInputArgument(ast, functionId);
				}
				Style::PopStyleCompact();

				UI::TableNextColumn();
				if (auto* children = AST::Hierarchy::GetChildren(ast, functionId))
				{
					auto exprInputs = ast.Filter<CExpressionInput>();
					for (AST::Id childId : *children)
					{
						if (exprInputs.Has(childId))
						{
							DrawField(ast, editor, functionId, childId);
						}
					}
				}
				Style::PushStyleCompact();
				if (UI::Button(ICON_FA_PLUS "##FunctionOutput", ImVec2(-FLT_MIN, 0.0f)))
				{
					ScopedChange(ast, functionId);
					AST::Functions::AddOutputArgument(ast, functionId);
				}
				Style::PopStyleCompact();
				UI::EndTable();
			}

			UI::Unindent(extraIndent);
		}
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
				bool showTable =
				    UI::BeginTable("##variableTable", 3, ImGuiTableFlags_SizingFixedFit);
				UI::PopStyleVar();
				if (showTable)
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
			}

			Style::PushStyleCompact();
			if (UI::Button(ICON_FA_PLUS "##Variable", ImVec2(-FLT_MIN, 0.0f)))
			{
				ScopedChange(ast, typeId);
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
				for (AST::Id child : *children)
				{
					if (functionView.Has(child))
					{
						DrawFunction(ast, editor, child);
					}
				}
			}

			Style::PushStyleCompact();
			if (UI::Button(ICON_FA_PLUS "##Function", ImVec2(-FLT_MIN, 0.0f)))
			{
				ScopedChange(ast, typeId);
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
