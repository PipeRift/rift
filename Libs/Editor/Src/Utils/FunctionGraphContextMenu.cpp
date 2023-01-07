// Copyright 2015-2023 Piperift - All rights reserved

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
#include <AST/Components/CNamespace.h>
#include <AST/Components/CType.h>
#include <AST/Components/Views/CNodePosition.h>
#include <AST/Statics/STypes.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TransactionUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <Pipe/ECS/Utils/Hierarchy.h>
#include <UI/UI.h>


namespace rift::Editor::Graph
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
					AST::TryConnectStmt(ast, linkPinId, id);
					AST::TryConnectExpr(ast, AST::GetExprOutputFromPin(ast, linkPinId),
					    AST::GetExprInputFromPin(ast, id));
					break;
				case Nodes::PinType::Input:
					AST::TryConnectStmt(ast, id, linkPinId);
					AST::TryConnectExpr(ast, AST::GetExprOutputFromPin(ast, id),
					    AST::GetExprInputFromPin(ast, linkPinId));
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
		const bool canEditBody = AST::CanEditFunctionBodies(ast, typeId);

		AST::Id firstNodeId = nodeIds[0];

		if (nodeIds.Size() == 1 && ast.Has<AST::CDeclFunction>(firstNodeId))
		{
			if (canEditBody && UI::MenuItem("Add return node"))
			{
				AST::Id newId = AST::AddReturn({ast, typeId});
				if (!IsNone(newId))
				{
					v2 position = ast.Get<CNodePosition>(firstNodeId).position;
					ast.Add<CNodePosition>(newId, position + v2{10.f, 0.f});

					AST::TryConnectStmt(ast, firstNodeId, newId);
				}
			}
		}
		TArray<AST::Id> calls = ecs::GetIf<AST::CExprCall>(ast, nodeIds);
		if (!calls.IsEmpty() && UI::MenuItem("Refresh"))
		{
			ast.AddN<AST::CCallDirty>(calls);
		}

		if (canEditBody && UI::MenuItem("Delete"))
		{
			AST::RemoveNodes(ast, nodeIds);
		}
	}

	void DrawLinksContextMenu(AST::Tree& ast, AST::Id typeId, TSpan<AST::Id> linkIds)
	{
		Check(!linkIds.IsEmpty());
		const bool canEditBody = AST::CanEditFunctionBodies(ast, typeId);

		AST::Id firstLinkId = linkIds[0];

		if (canEditBody && UI::MenuItem("Delete"))
		{
			ScopedChange(ast, linkIds);
			for (AST::Id linkId : linkIds)
			{
				AST::DisconnectExpr(ast, AST::GetExprInputFromPin(ast, firstLinkId));
				AST::DisconnectStmtLink(ast, firstLinkId);
			}
		}
	}

	void DrawGraphContextMenu(AST::Tree& ast, AST::Id typeId)
	{
		static ImGuiTextFilter filter;
		const bool canEditBody = AST::CanEditFunctionBodies(ast, typeId);

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
				AST::Id newId = AST::AddReturn({ast, typeId});
				SetPositionAndConnect(ast, newId, gridPos);
			}
			if (ContextItem("If", filter))
			{
				AST::Id newId = AST::AddIf({ast, typeId});
				SetPositionAndConnect(ast, newId, gridPos);
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Constructors", filter))
		{
			String makeStr{};
			auto& typeList = ast.GetStatic<AST::STypes>();
			TAccess<AST::CType, AST::CNamespace> typesAccess{ast};
			for (ecs::Id typeId : ecs::ListAll<AST::CType>(typesAccess))
			{
				if (auto* ns = typesAccess.TryGet<const AST::CNamespace>(typeId))
				{
					makeStr.clear();
					Strings::FormatTo(makeStr, "Make {}", ns->name);
					if (ContextItem(makeStr, filter))
					{
						AST::Id newId = AST::AddLiteral({ast, typeId}, typeId);
						SetPositionAndConnect(ast, newId, gridPos);
					}
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Functions", filter))
		{
			static String label;
			TAccess<AST::CDeclFunction, AST::CNamespace, AST::CChild, AST::CType> access{ast};
			for (AST::Id functionId : ecs::ListAll<AST::CDeclFunction, AST::CNamespace>(access))
			{
				Name name = access.Get<const AST::CNamespace>(functionId).name;
				label.clear();
				AST::Id funcTypeId = p::ecs::GetParent(access, functionId);
				if (!IsNone(funcTypeId) && access.Has<AST::CType, AST::CNamespace>(funcTypeId))
				{
					Strings::FormatTo(label, "{}   ({})", name,
					    access.Get<const AST::CNamespace>(funcTypeId).name);
				}
				else
				{
					Strings::FormatTo(label, "{}", name);
				}
				if (ContextItem(label, filter))
				{
					AST::Id newId = AST::AddCall({ast, typeId}, functionId);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Variables", filter))
		{
			static String label;
			TAccess<AST::CDeclVariable, AST::CNamespace, AST::CChild, AST::CType> access{ast};
			for (AST::Id variableId : ecs::ListAll<AST::CDeclVariable, AST::CNamespace>(access))
			{
				Name name = access.Get<const AST::CNamespace>(variableId).name;
				label.clear();
				AST::Id typeId = p::ecs::GetParent(access, variableId);
				if (!IsNone(typeId) && access.Has<AST::CType, AST::CNamespace>(typeId))
				{
					Strings::FormatTo(
					    label, "{}   ({})", name, access.Get<const AST::CNamespace>(typeId).name);
				}
				else
				{
					Strings::FormatTo(label, "{}", name);
				}
				if (ContextItem(label, filter))
				{
					AST::Id newId = AST::AddDeclarationReference({ast, typeId}, variableId);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Operators", filter))
		{
			static String name;
			// Unary operators
			for (auto type : GetEnumValues<AST::UnaryOperatorType>())
			{
				name.clear();
				StringView shortName = GetUnaryOperatorName(type);
				StringView longName  = GetUnaryOperatorLongName(type);
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (ContextItem(name, filter))
				{
					AST::Id newId = AST::AddUnaryOperator({ast, typeId}, type);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			// Binary operators
			for (auto type : GetEnumValues<AST::BinaryOperatorType>())
			{
				name.clear();
				StringView shortName = GetBinaryOperatorName(type);
				StringView longName  = GetBinaryOperatorLongName(type);
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (ContextItem(name, filter))
				{
					AST::Id newId = AST::AddBinaryOperator({ast, typeId}, type);
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
}    // namespace rift::Editor::Graph
