// Copyright 2015-2026 Piperift. All Rights Reserved.

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
#include <AST/Systems/FunctionsSystem.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <AST/Utils/TransactionUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <UI/UI.h>


namespace rift::editor::Graph
{
	// Expression pins carry a type, execution pins are the node itself and don't.
	static bool IsExprPin(ast::Tree& ast, ast::Id pinId)
	{
		return ast.Has<ast::CExprType>(pinId) || ast.Has<ast::CExprTypeId>(pinId);
	}

	static ast::Id GetPinTypeId(ast::Tree& ast, ast::Id pinId)
	{
		if (const auto* type = ast.TryGet<const ast::CExprTypeId>(pinId))
		{
			return type->id;
		}
		if (const auto* type = ast.TryGet<const ast::CExprType>(pinId))
		{
			return ast::FindIdFromNamespace(ast, type->type);
		}
		return ast::NoId;
	}

	static bool HasPinWithType(ast::Tree& ast, p::TView<const ast::Id> pinIds, ast::Id typeId)
	{
		for (ast::Id pinId : pinIds)
		{
			if (GetPinTypeId(ast, pinId) == typeId)
			{
				return true;
			}
		}
		return false;
	}

	// The first pin on 'nodeId' that the dropped link can connect to. For expression links
	// it's the first pin with a matching type, for execution links the node's exec pin.
	static ast::Id FindMatchingPin(ast::Tree& ast, ast::Id nodeId, const DroppedLink& dropped)
	{
		if (IsExprPin(ast, dropped.pinId))
		{
			const ast::Id typeId = GetPinTypeId(ast, dropped.pinId);
			if (dropped.pinType == Nodes::PinType::Output)
			{
				if (const auto* inputs = ast.TryGet<const ast::CExprInputs>(nodeId))
				{
					for (ast::Id pinId : inputs->pinIds)
					{
						if (GetPinTypeId(ast, pinId) == typeId)
						{
							return pinId;
						}
					}
				}
			}
			else
			{
				if (const auto* outputs = ast.TryGet<const ast::CExprOutputs>(nodeId))
				{
					for (ast::Id pinId : outputs->pinIds)
					{
						if (GetPinTypeId(ast, pinId) == typeId)
						{
							return pinId;
						}
					}
				}
			}
			return ast::NoId;
		}

		if (dropped.pinType == Nodes::PinType::Output && ast.Has<ast::CStmtInput>(nodeId))
		{
			return nodeId;
		}
		if (dropped.pinType == Nodes::PinType::Input)
		{
			if (ast.Has<ast::CStmtOutput>(nodeId))
			{
				return nodeId;
			}
			if (const auto* outputs = ast.TryGet<const ast::CStmtOutputs>(nodeId))
			{
				if (!outputs->pinIds.IsEmpty())
				{
					return outputs->pinIds[0];
				}
			}
		}
		return ast::NoId;
	}

	// Connects a freshly created node to the pin the user dropped over empty canvas.
	static void ConnectDroppedLink(ast::Tree& ast, ast::Id nodeId)
	{
		const DroppedLink dropped = GetDroppedLink();
		if (!dropped.IsValid())
		{
			return;
		}

		const ast::Id targetPin = FindMatchingPin(ast, nodeId, dropped);
		if (dropped.pinType == Nodes::PinType::Output)
		{
			ast::TryConnectStmt(ast, dropped.pinId, nodeId);
			if (!IsNone(targetPin))
			{
				ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, dropped.pinId),
				    ast::GetExprInputFromPin(ast, targetPin));
			}
		}
		else if (!IsNone(targetPin))
		{
			ast::TryConnectStmt(ast, targetPin, dropped.pinId);
			ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, targetPin),
			    ast::GetExprInputFromPin(ast, dropped.pinId));
		}
	}

	void SetPositionAndConnect(ast::Tree& ast, ast::Id id, v2 position)
	{
		if (IsNone(id))
		{
			return;
		}

		ast.Add<CNodePosition>(id, position);

		if (GetDroppedLink().IsValid())
		{
			ConnectDroppedLink(ast, id);
			GetDroppedLink() = {};
			return;
		}

		// TODO: Improve nodes input to handle this correctly
		TPair<Nodes::Id, Nodes::PinType> linkPin = Nodes::GetDraggedOriginPin();

		const auto linkPinId = ast::Id::MakeRaw(linkPin.first);
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

	static bool DroppedLinkFitsLiteral(ast::Tree& ast, ast::Id literalTypeId)
	{
		const DroppedLink& dropped = GetDroppedLink();
		if (!dropped.IsValid())
		{
			return true;
		}
		// Literals only produce a value, so they can only receive an input link
		if (!IsExprPin(ast, dropped.pinId) || dropped.pinType != Nodes::PinType::Input)
		{
			return false;
		}
		return literalTypeId == GetPinTypeId(ast, dropped.pinId);
	}

	static bool DroppedLinkFitsReturn(ast::Tree& ast)
	{
		const DroppedLink& dropped = GetDroppedLink();
		if (!dropped.IsValid())
		{
			return true;
		}
		// Return nodes take execution flow; their value inputs come from the function
		return !IsExprPin(ast, dropped.pinId) && dropped.pinType == Nodes::PinType::Output;
	}

	static bool DroppedLinkFitsIf(ast::Tree& ast)
	{
		const DroppedLink& dropped = GetDroppedLink();
		if (!dropped.IsValid())
		{
			return true;
		}
		if (!IsExprPin(ast, dropped.pinId))
		{
			return true;    // if nodes have both an execution input and outputs
		}
		// Only a bool output fits the if node's condition input
		return dropped.pinType == Nodes::PinType::Output
		    && GetPinTypeId(ast, dropped.pinId) == ast.GetNativeTypes().boolId;
	}

	static bool DroppedLinkFitsFunction(ast::Tree& ast, ast::Id functionId)
	{
		const DroppedLink& dropped = GetDroppedLink();
		if (!dropped.IsValid() || !IsExprPin(ast, dropped.pinId))
		{
			return true;    // call nodes always have an execution input and output
		}

		const ast::Id typeId = GetPinTypeId(ast, dropped.pinId);
		if (dropped.pinType == Nodes::PinType::Output)
		{
			// Call inputs mirror the function's parameters
			if (const auto* params = ast.TryGet<const ast::CExprOutputs>(functionId))
			{
				return HasPinWithType(ast, params->pinIds, typeId);
			}
		}
		else if (const auto* returns = ast.TryGet<const ast::CExprInputs>(functionId))
		{
			// Call outputs mirror the function's return values
			return HasPinWithType(ast, returns->pinIds, typeId);
		}
		return false;
	}

	static bool DroppedLinkFitsVariable(ast::Tree& ast, ast::Id variableId)
	{
		const DroppedLink& dropped = GetDroppedLink();
		if (!dropped.IsValid())
		{
			return true;
		}
		if (!IsExprPin(ast, dropped.pinId) || dropped.pinType != Nodes::PinType::Input)
		{
			return false;
		}
		const auto* variable = ast.TryGet<const ast::CDeclVariable>(variableId);
		if (!variable || IsNone(variable->typeId))
		{
			return false;
		}
		return variable->typeId == GetPinTypeId(ast, dropped.pinId);
	}

	static bool DroppedLinkFitsOperator(ast::Tree& ast)
	{
		const DroppedLink& dropped = GetDroppedLink();
		// Operator pin types are inferred from their connections, so accept any expression
		return !dropped.IsValid() || IsExprPin(ast, dropped.pinId);
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
			    || (UI::IsItemFocused() && UI::IsKeyPressed(ImGuiKey_Enter)))
			{
				UI::CloseCurrentPopup();
				return true;
			}
		}
		return false;
	}

	void DrawNodesContextMenu(ast::Tree& ast, ast::Id typeId, TView<ast::Id> nodeIds)
	{
		P_Check(!nodeIds.IsEmpty());
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
			ast.AddN<CMdfd<ast::CExprCallId>>(calls);
		}
	}

	void DrawLinksContextMenu(ast::Tree& ast, ast::Id typeId, TView<ast::Id> linkIds)
	{
		P_Check(!linkIds.IsEmpty());
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

	struct ContextFunction
	{
		ast::Id id;
		ast::Namespace ns;    // Full namespace, function name included as its last scope
	};

	// Draws functions grouped by their namespace. 'depth' is the namespace scope being
	// grouped at, so the root scope becomes a root category and the type nests under it.
	static void DrawFunctionNamespace(ast::Tree& ast, ast::Id typeId,
	    const TArray<ContextFunction>& functions, i32 depth, const ImGuiTextFilter& filter,
	    v2 gridPos)
	{
		TArray<Tag> drawnCategories;
		for (const ContextFunction& function : functions)
		{
			const i32 functionDepth = function.ns.Size() - 1;
			if (functionDepth <= depth)
			{
				continue;    // The function itself is drawn at this level
			}

			const Tag category = function.ns[depth];
			if (drawnCategories.Contains(category))
			{
				continue;
			}
			drawnCategories.Add(category);

			TArray<ContextFunction> categoryFunctions;
			for (const ContextFunction& other : functions)
			{
				if (other.ns.Size() - 1 > depth && other.ns[depth] == category)
				{
					categoryFunctions.Add(other);
				}
			}

			if (ContextTreeNode(category.AsString(), filter))
			{
				DrawFunctionNamespace(ast, typeId, categoryFunctions, depth + 1, filter, gridPos);
				ContextTreePop(filter);
			}
		}

		for (const ContextFunction& function : functions)
		{
			if (function.ns.Size() - 1 != depth)
			{
				continue;
			}

			const Tag functionName = function.ns[depth];
			if (DroppedLinkFitsFunction(ast, function.id)
			    && ContextItem(functionName.AsString(), filter))
			{
				ast::Id newId = ast::AddCall({ast, typeId}, function.id);
				// Populate the new call's pins right away so the dropped link can find a
				// matching pin to connect to
				ast::FunctionsSystem::SyncCallPinsFromFunction(ast);
				SetPositionAndConnect(ast, newId, gridPos);
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
			if (DroppedLinkFitsReturn(ast) && ContextItem("Return", filter))
			{
				ast::Id newId = ast::AddReturn({ast, typeId});
				SetPositionAndConnect(ast, newId, gridPos);
			}
			if (DroppedLinkFitsIf(ast) && ContextItem("If", filter))
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
			TIdScope<ast::CDeclType, ast::CNamespace> typesScope{ast};
			for (Id typeId : FindAllIdsWith<ast::CDeclType>(typesScope))
			{
				if (auto* ns = typesScope.TryGet<const ast::CNamespace>(typeId))
				{
					makeStr.clear();
					p::FormatTo(makeStr, "Make {}", ns->name);
					if (DroppedLinkFitsLiteral(ast, typeId) && ContextItem(makeStr, filter))
					{
						ast::Id newId = ast::AddLiteral({ast, typeId}, typeId);
						SetPositionAndConnect(ast, newId, gridPos);
					}
				}
			}
			ContextTreePop(filter);
		}

		{
			TIdScope<ast::CDeclFunction, ast::CNamespace, ast::CChild, ast::CModule> scope{ast};
			TArray<ContextFunction> functions;
			for (ast::Id functionId : FindAllIdsWith<ast::CDeclFunction, ast::CNamespace>(scope))
			{
				functions.Add({functionId, ast::GetNamespace(scope, functionId)});
			}
			if (!functions.IsEmpty())
			{
				DrawFunctionNamespace(ast, typeId, functions, 0, filter, gridPos);
			}
		}

		if (ContextTreeNode("Variables", filter))
		{
			static String label;
			TIdScope<ast::CDeclVariable, ast::CNamespace, ast::CChild, ast::CDeclType> scope{ast};
			for (ast::Id variableId : FindAllIdsWith<ast::CDeclVariable, ast::CNamespace>(scope))
			{
				Tag name = scope.Get<const ast::CNamespace>(variableId).name;
				label.clear();
				ast::Id typeId = p::GetIdParent(scope, variableId);
				if (!IsNone(typeId) && scope.Has<ast::CDeclType, ast::CNamespace>(typeId))
				{
					p::FormatTo(
					    label, "{}   ({})", name, scope.Get<const ast::CNamespace>(typeId).name);
				}
				else
				{
					p::FormatTo(label, "{}", name);
				}
				if (DroppedLinkFitsVariable(ast, variableId) && ContextItem(label, filter))
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
				p::FormatTo(name, "{}   ({})", shortName, longName);
				if (DroppedLinkFitsOperator(ast) && ContextItem(name, filter))
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
				p::FormatTo(name, "{}   ({})", shortName, longName);
				if (DroppedLinkFitsOperator(ast) && ContextItem(name, filter))
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
}    // namespace rift::editor::Graph
