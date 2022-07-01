// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/FunctionGraphContextMenu.h"

#include "AST/Utils/TypeUtils.h"
#include "UI/Style.h"
#include "UI/Widgets.h"
#include "Utils/EditorStyle.h"
#include "Utils/FunctionGraph.h"
#include "Utils/Nodes.h"
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


namespace rift::Graph
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

	void DrawNodesContextMenu(AST::Tree& ast, AST::Id typeId, TSpan<AST::Id> nodeIds)
	{
		Check(!nodeIds.IsEmpty());
		const bool canEditBody = Types::CanEditFunctionBodies(ast, typeId);

		AST::Id firstNodeId = nodeIds[0];

		if (nodeIds.Size() == 1 && ast.Has<CDeclFunction>(firstNodeId))
		{
			if (canEditBody && UI::MenuItem("Add return node"))
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
		TArray<AST::Id> calls = ecs::GetIf<CExprCall>(ast, nodeIds);
		if (!calls.IsEmpty() && UI::MenuItem("Refresh"))
		{
			ast.Add<CCallDirty>(calls);
		}

		if (canEditBody && UI::MenuItem("Delete"))
		{
			Types::RemoveNodes(ast, nodeIds);
		}
	}

	void DrawLinksContextMenu(AST::Tree& ast, AST::Id typeId, TSpan<AST::Id> linkIds)
	{
		Check(!linkIds.IsEmpty());
		const bool canEditBody = Types::CanEditFunctionBodies(ast, typeId);

		AST::Id firstLinkId = linkIds[0];

		if (canEditBody && UI::MenuItem("Delete"))
		{
			for (AST::Id linkId : linkIds)
			{
				AST::Expressions::Disconnect(
				    ast, AST::Expressions::InputFromPinId(ast, firstLinkId));
				AST::Statements::DisconnectFromInputPin(ast, firstLinkId);
			}
		}
	}

	void DrawGraphContextMenu(AST::Tree& ast, AST::Id typeId)
	{
		static ImGuiTextFilter filter;
		const bool canEditBody = Types::CanEditFunctionBodies(ast, typeId);

		if (UI::IsWindowAppearing())
		{
			UI::SetKeyboardFocusHere();
			filter.Clear();
		}
		filter.Draw("##Filter");
		const v2 clickPos = UI::GetMousePosOnOpeningCurrentPopup();
		const v2 gridPos  = settings.GetGridPosition(clickPos).Floor();

		if (!canEditBody)
		{
			return;
		}

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
			TAccess<CType> typesAccess{ast};
			for (const auto& it : typeList.typesByName)
			{
				if (auto* type = typesAccess.TryGet<const CType>(it.second))
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
			for (AST::Id functionId : ecs::ListAll<CDeclFunction, CIdentifier>(access))
			{
				Name name = access.Get<const CIdentifier>(functionId).name;
				label.clear();
				AST::Id funcTypeId = AST::Hierarchy::GetParent(access, functionId);
				if (!IsNone(funcTypeId) && access.Has<CType>(funcTypeId))
				{
					Strings::FormatTo(
					    label, "{}   ({})", name, access.Get<const CType>(funcTypeId).name);
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
			for (AST::Id variableId : ecs::ListAll<CDeclVariable, CIdentifier>(access))
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
			for (auto type : GetEnumValues<UnaryOperatorType>())
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
			for (auto type : GetEnumValues<BinaryOperatorType>())
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

	void DrawLinkContextMenu() {}

	void DrawContextMenu(
	    AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId, AST::Id hoveredLinkId)
	{
		if (UI::BeginPopup("ContextMenu"))
		{
			if (!IsNone(hoveredNodeId))
			{
				if (Nodes::IsNodeSelected(hoveredNodeId))
				{
					DrawNodesContextMenu(ast, typeId, Nodes::GetSelectedNodes());
				}
				else
				{
					DrawNodesContextMenu(ast, typeId, hoveredNodeId);
				}
			}
			else if (!IsNone(hoveredLinkId))
			{
				TArray<AST::Id> selectedLinkIds;
				if (Nodes::GetSelectedLinks(selectedLinkIds))
				{
					DrawLinksContextMenu(ast, typeId, selectedLinkIds);
				}
				else
				{
					DrawLinksContextMenu(ast, typeId, hoveredLinkId);
				}
			}
			else
			{
				DrawGraphContextMenu(ast, typeId);
			}
			UI::EndPopup();
		}
	}
}    // namespace rift::Graph
