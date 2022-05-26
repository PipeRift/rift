// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/FunctionGraphContextMenu.h"

#include "UI/Style.h"
#include "UI/Widgets.h"
#include "Utils/EditorStyle.h"
#include "Utils/FunctionGraph.h"
#include "Utils/TypeUtils.h"
#include "Utils/Widgets.h"

#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprType.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CType.h>
#include <AST/Components/Views/CNodePosition.h>
#include <AST/Statics/STypes.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TransactionUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <UI/UI.h>


namespace Rift::Graph
{
	void SetPositionAndConnect(AST::Tree& ast, AST::Id id, v2 position)
	{
		if (!IsNone(id))
		{
			ast.Add<CNodePosition>(id, position);
			// TODO: Improve nodes input to handle this correctly
			TPair<Nodes::Id, Nodes::PinType> linkPin = Nodes::GetDraggedOriginPin();

			const auto linkPinId = AST::Id(linkPin.first);
			switch (linkPin.second)
			{
				case Nodes::PinType::Output:
					AST::Statements::TryConnect(ast, linkPinId, id);
					AST::Expressions::TryConnect(ast,
					    AST::Expressions::OutputFromPinId(ast, linkPinId),
					    AST::Expressions::InputFromPinId(ast, id));
					break;
				case Nodes::PinType::Input:
					AST::Statements::TryConnect(ast, id, linkPinId);
					AST::Expressions::TryConnect(ast, AST::Expressions::OutputFromPinId(ast, id),
					    AST::Expressions::InputFromPinId(ast, linkPinId));
					break;
				default: break;
			}
		}
	}

	bool ContextTreeNode(StringView name, const ImGuiTextFilter& filter)
	{
		return filter.IsActive() || UI::TreeNode(name.data());
	}
	void ContextTreePop(const ImGuiTextFilter& filter)
	{
		if (!filter.IsActive())
		{
			UI::TreePop();
		}
	}

	bool ContextItem(StringView name, const ImGuiTextFilter& filter)
	{
		if (filter.PassFilter(name.data(), name.data() + name.size()))
		{
			// Return true also if Enter is pressed
			if (UI::MenuItem(name.data())
			    || (UI::IsItemFocused() && UI::IsKeyPressedMap(ImGuiKey_Enter)))
			{
				UI::CloseCurrentPopup();
				return true;
			}
		}
		return false;
	}

	void EditFunctionPin(AST::Tree& ast, AST::Id typeId, AST::Id id)
	{
		auto* identifier = ast.TryGet<CIdentifier>(id);
		auto* type       = ast.TryGet<CExprType>(id);
		if (!identifier || !type)
		{
			return;
		}

		bool removePin = false;
		bool hovered   = false;

		static String labelId;
		static String popupName;
		popupName.clear();
		Strings::FormatTo(popupName, "##PinContextMenu_{}", id);


		UI::TableNextRow();
		const Color color = Style::GetTypeColor(ast, type->id);
		UI::TableSetBgColor(ImGuiTableBgTarget_RowBg0, color.DWColor());

		UI::TableNextColumn();    // Name
		labelId.clear();
		Strings::FormatTo(labelId, "##Name_{}", id);
		String name = identifier->name.ToString();
		UI::SetNextItemWidth(UI::GetContentRegionAvail().x);
		if (UI::MutableText(labelId, name, ImGuiInputTextFlags_AutoSelectAll))
		{
			ScopedChange(ast, id);
			identifier->name = Name{name};
		}
		if (UI::IsItemHovered())
		{
			hovered = true;
		}

		UI::TableNextColumn();    // Type
		UI::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
		labelId.clear();
		Strings::FormatTo(labelId, "##Type_{}", id);
		AST::Id selectedTypeId = type->id;
		UI::SetNextItemWidth(-FLT_MIN);
		if (Editor::TypeCombo(ast, labelId, selectedTypeId))
		{
			ScopedChange(ast, id);
			type->id = selectedTypeId;
		}
		UI::PopStyleVar();
		if (UI::IsItemHovered())
		{
			hovered = true;
		}

		if (hovered)
		{
			if (UI::IsKeyReleased(GLFW_KEY_DELETE))
			{
				removePin = true;
			}
			else if (UI::IsMouseReleased(ImGuiMouseButton_Right))
			{
				UI::OpenPopup(popupName.c_str());
			}
		}
		if (UI::BeginPopup(popupName.c_str()))
		{
			if (UI::MenuItem("Delete"))
			{
				removePin = true;
			}
			UI::EndPopup();
		}
		if (removePin)
		{
			AST::Expressions::RemoveInputPin(ast, AST::Expressions::InputFromPinId(ast, id));
			AST::Expressions::RemoveOutputPin(ast, AST::Expressions::OutputFromPinId(ast, id));
		}
	}

	void EditFunction(AST::Tree& ast, AST::Id typeId, AST::Id id)
	{
		auto* identifier = ast.TryGet<CIdentifier>(id);
		if (!identifier)
		{
			return;
		}

		String functionName = identifier->name.ToString();
		UI::SetNextItemWidth(UI::GetContentRegionAvail().x);
		if (UI::InputText("##name", functionName, ImGuiInputTextFlags_AutoSelectAll))
		{
			ScopedChange(ast, id);
			identifier->name = Name{functionName};
		}
		UI::Spacing();

		UI::Text("Inputs");
		if (UI::BeginTable("##fields", 2, ImGuiTableFlags_SizingFixedFit, {200.f, 0.f}))
		{
			UI::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.9f);
			UI::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.f);
			if (const auto* exprOutputs = ast.TryGet<const CExprOutputs>(id))
			{
				for (AST::Id pinId : exprOutputs->pinIds)
				{
					EditFunctionPin(ast, id, pinId);
				}
			}
			UI::EndTable();
		}
		Style::PushStyleCompact();
		UI::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5f, 0.5f});
		UI::SetNextItemWidth(UI::GetContentRegionAvailWidth());
		if (UI::Selectable(ICON_FA_PLUS "##AddInput"))
		{
			ScopedChange(ast, id);
			Types::AddFunctionInput(ast, id);
		}
		UI::HelpTooltip("Adds a new input parameter to a function");
		UI::PopStyleVar();
		Style::PopStyleCompact();
		UI::Spacing();

		UI::Text("Outputs");
		if (UI::BeginTable("##fields", 2, ImGuiTableFlags_SizingFixedFit, {200.f, 0.f}))
		{
			UI::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.9f);
			UI::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.f);
			if (const auto* exprInputs = ast.TryGet<const CExprInputs>(id))
			{
				for (AST::Id pinId : exprInputs->pinIds)
				{
					EditFunctionPin(ast, id, pinId);
				}
			}
			UI::EndTable();
		}
		Style::PushStyleCompact();
		UI::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
		UI::SetNextItemWidth(UI::GetContentRegionAvailWidth());
		if (UI::Selectable(ICON_FA_PLUS "##AddOutput"))
		{
			ScopedChange(ast, id);
			Types::AddFunctionOutput(ast, id);
		}
		UI::HelpTooltip("Adds a new output parameter to a function");
		UI::PopStyleVar();
		Style::PopStyleCompact();
		UI::Spacing();
	}

	void DrawNodesContextMenu(AST::Tree& ast, AST::Id typeId, TSpan<AST::Id> nodeIds)
	{
		Check(!nodeIds.IsEmpty());

		AST::Id firstNodeId = nodeIds[0];

		if (nodeIds.Size() == 1 && ast.Has<CDeclFunction>(firstNodeId))
		{
			EditFunction(ast, typeId, nodeIds[0]);
			UI::Separator();

			if (UI::MenuItem("Add return node"))
			{
				AST::Id newId = Types::AddReturn({ast, typeId});
				if (!IsNone(newId))
				{
					v2 position = ast.Get<CNodePosition>(firstNodeId).position;
					ast.Add<CNodePosition>(newId, position + v2{10.f, 0.f});

					AST::Statements::TryConnect(ast, firstNodeId, newId);
				}
			}
		}
		TArray<AST::Id> calls = AST::GetIf<CExprCall>(ast, nodeIds);
		if (!calls.IsEmpty() && UI::MenuItem("Refresh"))
		{
			ast.Add<CCallDirty>(calls);
		}

		if (UI::MenuItem("Delete"))
		{
			Types::RemoveNodes(ast, nodeIds);
		}
	}

	void DrawGraphContextMenu(AST::Tree& ast, AST::Id typeId)
	{
		static ImGuiTextFilter filter;

		if (UI::IsWindowAppearing())
		{
			UI::SetKeyboardFocusHere();
			filter.Clear();
		}
		filter.Draw("##Filter");
		const v2 clickPos = UI::GetMousePosOnOpeningCurrentPopup();
		const v2 gridPos  = settings.GetGridPosition(clickPos).Floor();

		if (ContextTreeNode("Flow", filter))
		{
			if (ContextItem("Return", filter))
			{
				AST::Id newId = Types::AddReturn({ast, typeId});
				SetPositionAndConnect(ast, newId, gridPos);
			}
			if (ContextItem("If", filter))
			{
				AST::Id newId = Types::AddIf({ast, typeId});
				SetPositionAndConnect(ast, newId, gridPos);
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Constructors", filter))
		{
			String makeStr{};
			auto& typeList = ast.GetStatic<STypes>();
			auto types     = ast.Filter<CType>();
			for (const auto& it : typeList.typesByName)
			{
				if (auto* type = types.TryGet<CType>(it.second))
				{
					makeStr.clear();
					Strings::FormatTo(makeStr, "Make {}", type->name);
					if (ContextItem(makeStr, filter))
					{
						AST::Id newId = Types::AddLiteral({ast, typeId}, it.second);
						SetPositionAndConnect(ast, newId, gridPos);
					}
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Functions", filter))
		{
			static String label;
			TAccess<CDeclFunction, CIdentifier, CChild, CType> access{ast};
			for (AST::Id functionId : AST::ListAll<CDeclFunction, CIdentifier>(access))
			{
				Name name = access.Get<const CIdentifier>(functionId).name;
				label.clear();
				AST::Id typeId = AST::Hierarchy::GetParent(access, functionId);
				if (!IsNone(typeId) && access.Has<CType>(typeId))
				{
					Strings::FormatTo(
					    label, "{}   ({})", name, access.Get<const CType>(typeId).name);
				}
				else
				{
					Strings::FormatTo(label, "{}", name);
				}
				if (ContextItem(label, filter))
				{
					AST::Id newId = Types::AddCall({ast, typeId}, functionId);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Variables", filter))
		{
			static String label;
			TAccess<CDeclVariable, CIdentifier, CChild, CType> access{ast};
			for (AST::Id variableId : AST::ListAll<CDeclVariable, CIdentifier>(access))
			{
				Name name = access.Get<const CIdentifier>(variableId).name;
				label.clear();
				AST::Id typeId = AST::Hierarchy::GetParent(access, variableId);
				if (!IsNone(typeId) && access.Has<CType>(typeId))
				{
					Strings::FormatTo(
					    label, "{}   ({})", name, access.Get<const CType>(typeId).name);
				}
				else
				{
					Strings::FormatTo(label, "{}", name);
				}
				if (ContextItem(label, filter))
				{
					AST::Id newId = Types::AddDeclarationReference({ast, typeId}, variableId);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Operators", filter))
		{
			static String name;
			// Unary operators
			for (auto type : Refl::GetEnumValues<UnaryOperatorType>())
			{
				name.clear();
				StringView shortName = Types::GetUnaryOperatorName(type);
				StringView longName  = Types::GetUnaryOperatorLongName(type);
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (ContextItem(name, filter))
				{
					AST::Id newId = Types::AddUnaryOperator({ast, typeId}, type);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			// Binary operators
			for (auto type : Refl::GetEnumValues<BinaryOperatorType>())
			{
				name.clear();
				StringView shortName = Types::GetBinaryOperatorName(type);
				StringView longName  = Types::GetBinaryOperatorLongName(type);
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (ContextItem(name, filter))
				{
					AST::Id newId = Types::AddBinaryOperator({ast, typeId}, type);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			ContextTreePop(filter);
		}
	}

	void DrawContextMenu(AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId)
	{
		if (UI::BeginPopup("ContextMenu"))
		{
			if (IsNone(hoveredNodeId))
			{
				DrawGraphContextMenu(ast, typeId);
			}
			else if (Nodes::IsNodeSelected(hoveredNodeId))
			{
				DrawNodesContextMenu(ast, typeId, Nodes::GetSelectedNodes());
			}
			else
			{
				DrawNodesContextMenu(ast, typeId, hoveredNodeId);
			}
			UI::EndPopup();
		}
	}
}    // namespace Rift::Graph
