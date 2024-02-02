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

#include <AST/Components/CNamespace.h>
#include <AST/Components/Declarations.h>
#include <AST/Components/Expressions.h>
#include <AST/Components/Views/CNodePosition.h>
#include <AST/Statics/STypes.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TransactionUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <UI/UI.h>


namespace rift::Editor::Graph
{
	void SetPositionAndConnect(ast::Tree& ast, ast::Id id, v2 position)
	{
		if (!IsNone(id))
		{
			ast.Add<CNodePosition>(id, position);
			// TODO: Improve nodes input to handle this correctly
			TPair<Nodes::Id, Nodes::PinType> linkPin = Nodes::GetDraggedOriginPin();

			const auto linkPinId = ast::Id(linkPin.first);
			switch (linkPin.second)
			{
				case Nodes::PinType::Output:
					ast::TryConnectStmt(ast, linkPinId, id);
					ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, linkPinId),
					    ast::GetExprInputFromPin(ast, id));
					break;
				case Nodes::PinType::Input:
					ast::TryConnectStmt(ast, id, linkPinId);
					ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, id),
					    ast::GetExprInputFromPin(ast, linkPinId));
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

	void DrawNodesContextMenu(ast::Tree& ast, ast::Id typeId, TView<ast::Id> nodeIds)
	{
		Check(!nodeIds.IsEmpty());
		const bool canEditBody = ast::HasFunctionBodies(ast, typeId);

		if (canEditBody && UI::MenuItem("Delete"))
		{
			ast::RemoveNodes(ast, nodeIds);
		}

		ast::Id firstNodeId = nodeIds[0];
		if (nodeIds.Size() == 1 && ast.Has<ast::CDeclFunction>(firstNodeId))
		{
			if (canEditBody && UI::MenuItem("Add return node"))
			{
				ast::Id newId = ast::AddReturn({ast, typeId});
				if (!IsNone(newId))
				{
					v2 position = ast.Get<CNodePosition>(firstNodeId).position;
					ast.Add<CNodePosition>(newId, position + v2{10.f, 0.f});

					ast::TryConnectStmt(ast, firstNodeId, newId);
				}
			}
		}
		TArray<ast::Id> calls = FindIdsWith<ast::CExprCall>(ast, nodeIds);
		if (!calls.IsEmpty() && UI::MenuItem("Refresh"))
		{
			ast.AddN<ast::CCallDirty>(calls);
		}
	}

	void DrawLinksContextMenu(ast::Tree& ast, ast::Id typeId, TView<ast::Id> linkIds)
	{
		Check(!linkIds.IsEmpty());
		const bool canEditBody = ast::HasFunctionBodies(ast, typeId);

		ast::Id firstLinkId = linkIds[0];

		if (canEditBody && UI::MenuItem("Delete"))
		{
			ScopedChange(ast, linkIds);
			for (ast::Id linkId : linkIds)
			{
				ast::DisconnectExpr(ast, ast::GetExprInputFromPin(ast, firstLinkId));
				ast::DisconnectStmtLink(ast, firstLinkId);
			}
		}
	}

	void DrawGraphContextMenu(ast::Tree& ast, ast::Id typeId)
	{
		static ImGuiTextFilter filter;
		const bool canEditBody = ast::HasFunctionBodies(ast, typeId);

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
				ast::Id newId = ast::AddReturn({ast, typeId});
				SetPositionAndConnect(ast, newId, gridPos);
			}
			if (ContextItem("If", filter))
			{
				ast::Id newId = ast::AddIf({ast, typeId});
				SetPositionAndConnect(ast, newId, gridPos);
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Constructors", filter))
		{
			String makeStr{};
			auto& typeList = ast.GetStatic<ast::STypes>();
			TAccess<ast::CDeclType, ast::CNamespace> typesAccess{ast};
			for (Id typeId : FindAllIdsWith<ast::CDeclType>(typesAccess))
			{
				if (auto* ns = typesAccess.TryGet<const ast::CNamespace>(typeId))
				{
					makeStr.clear();
					Strings::FormatTo(makeStr, "Make {}", ns->name);
					if (ContextItem(makeStr, filter))
					{
						ast::Id newId = ast::AddLiteral({ast, typeId}, typeId);
						SetPositionAndConnect(ast, newId, gridPos);
					}
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Functions", filter))
		{
			static String label;
			TAccess<ast::CDeclFunction, ast::CNamespace, ast::CChild, ast::CDeclType> access{ast};
			for (ast::Id functionId : FindAllIdsWith<ast::CDeclFunction, ast::CNamespace>(access))
			{
				Tag name = access.Get<const ast::CNamespace>(functionId).name;
				label.clear();
				ast::Id funcTypeId = p::GetIdParent(access, functionId);
				if (!IsNone(funcTypeId) && access.Has<ast::CDeclType, ast::CNamespace>(funcTypeId))
				{
					Strings::FormatTo(label, "{}   ({})", name,
					    access.Get<const ast::CNamespace>(funcTypeId).name);
				}
				else
				{
					Strings::FormatTo(label, "{}", name);
				}
				if (ContextItem(label, filter))
				{
					ast::Id newId = ast::AddCall({ast, typeId}, functionId);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Variables", filter))
		{
			static String label;
			TAccess<ast::CDeclVariable, ast::CNamespace, ast::CChild, ast::CDeclType> access{ast};
			for (ast::Id variableId : FindAllIdsWith<ast::CDeclVariable, ast::CNamespace>(access))
			{
				Tag name = access.Get<const ast::CNamespace>(variableId).name;
				label.clear();
				ast::Id typeId = p::GetIdParent(access, variableId);
				if (!IsNone(typeId) && access.Has<ast::CDeclType, ast::CNamespace>(typeId))
				{
					Strings::FormatTo(
					    label, "{}   ({})", name, access.Get<const ast::CNamespace>(typeId).name);
				}
				else
				{
					Strings::FormatTo(label, "{}", name);
				}
				if (ContextItem(label, filter))
				{
					ast::Id newId = ast::AddDeclarationReference({ast, typeId}, variableId);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Operators", filter))
		{
			static String name;
			// Unary operators
			for (auto type : GetEnumValues<ast::UnaryOperatorType>())
			{
				name.clear();
				StringView shortName = GetUnaryOperatorName(type);
				StringView longName  = GetUnaryOperatorLongName(type);
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (ContextItem(name, filter))
				{
					ast::Id newId = ast::AddUnaryOperator({ast, typeId}, type);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			// Binary operators
			for (auto type : GetEnumValues<ast::BinaryOperatorType>())
			{
				name.clear();
				StringView shortName = GetBinaryOperatorName(type);
				StringView longName  = GetBinaryOperatorLongName(type);
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (ContextItem(name, filter))
				{
					ast::Id newId = ast::AddBinaryOperator({ast, typeId}, type);
					SetPositionAndConnect(ast, newId, gridPos);
				}
			}
			ContextTreePop(filter);
		}
	}

	void DrawLinkContextMenu() {}

	void DrawContextMenu(
	    ast::Tree& ast, ast::Id typeId, ast::Id hoveredNodeId, ast::Id hoveredLinkId)
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
				TArray<ast::Id> selectedLinkIds;
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
