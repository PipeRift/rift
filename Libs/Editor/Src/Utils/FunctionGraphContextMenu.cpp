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
#include <AST/Systems/ConversionSystem.h>
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
	static bool IsExprPin(ast::Tree& ast, Id pinId)
	{
		return ast.Has<ast::CExprType>(pinId) || ast.Has<ast::CExprTypeId>(pinId);
	}

	// Can a value of 'valueType' be used where a value of 'targetType' is expected?
	static bool IsTypeCompatible(ast::Tree& ast, Id valueType, Id targetType)
	{
		if (valueType == targetType)
		{
			return true;
		}
		// Pins without a type take the type of whatever gets connected to them
		if (IsNone(valueType) || IsNone(targetType))
		{
			return true;
		}
		return ast::ConversionSystem::CanConvertImplicitly(ast, valueType, targetType);
	}

	// The first pin on 'nodeId' that the dropped link can connect to. For expression links
	// it's the first pin with a matching type, for execution links the node's exec pin.
	static Id FindMatchingPin(ast::Tree& ast, Id nodeId, const MenuContext& context)
	{
		const Id sourceType = GetPinType(ast, context.sourcePin);
		if (!IsNone(sourceType))
		{
			// Pins of a compatible type are used when no pin matches exactly
			Id compatiblePin = ast::NoId;
			if (context.type == Nodes::PinType::Output)
			{
				if (const auto* inputs = ast.TryGet<const ast::CExprInputs>(nodeId))
				{
					for (Id pinId : inputs->pinIds)
					{
						const Id pinType = GetPinType(ast, pinId);
						if (pinType == sourceType)
						{
							return pinId;
						}
						if (IsNone(compatiblePin) && IsTypeCompatible(ast, sourceType, pinType))
						{
							compatiblePin = pinId;
						}
					}
				}
			}
			else
			{
				if (const auto* outputs = ast.TryGet<const ast::CExprOutputs>(nodeId))
				{
					for (Id pinId : outputs->pinIds)
					{
						const Id pinType = GetPinType(ast, pinId);
						if (pinType == sourceType)
						{
							return pinId;
						}
						if (IsNone(compatiblePin) && IsTypeCompatible(ast, pinType, sourceType))
						{
							compatiblePin = pinId;
						}
					}
				}
			}
			return compatiblePin;
		}

		if (context.type == Nodes::PinType::Output && ast.Has<ast::CStmtInput>(nodeId))
		{
			return nodeId;
		}
		if (context.type == Nodes::PinType::Input)
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

	void SetPositionAndConnect(ast::Tree& ast, Id id, const MenuContext& context)
	{
		if (IsNone(id))
		{
			return;
		}

		ast.Add<CNodePosition>(id, settings.GetGridPosition(context.location).Floor());

		if (context.HasSource())
		{
			const Id targetPin = FindMatchingPin(ast, id, context);
			if (context.type == Nodes::PinType::Output)
			{
				ast::TryConnectStmt(ast, context.sourcePin, id);
				if (!IsNone(targetPin))
				{
					ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, context.sourcePin),
					    ast::GetExprInputFromPin(ast, targetPin));
				}
			}
			else if (!IsNone(targetPin))
			{
				ast::TryConnectStmt(ast, targetPin, context.sourcePin);
				ast::TryConnectExpr(ast, ast::GetExprOutputFromPin(ast, targetPin),
				    ast::GetExprInputFromPin(ast, context.sourcePin));
			}
		}
	}

	enum class ContextItemKind
	{
		Literal,
		Return,
		If,
		Function,
		Variable,
		Operator,
	};

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

	bool MatchesContext(ast::Tree& ast, const MenuContext& context, ContextItemKind kind,
	    StringView name, const ImGuiTextFilter& filter, Id itemId = ast::NoId)
	{
		if (!filter.PassFilter(name.data(), name.data() + name.size()))
		{
			return false;
		}

		if (!context.HasSource())
		{
			return true;
		}

		const bool isExpr   = IsExprPin(ast, context.sourcePin);
		const bool isOutput = context.type == Nodes::PinType::Output;
		const Id sourceType = GetPinType(ast, context.sourcePin);

		switch (kind)
		{
			case ContextItemKind::Function:
				if (!isExpr)    // call nodes always have an stmt input and output
				{
					return true;
				}
				else if (isOutput)
				{
					// Input matches type?
					if (const auto* inputs = ast.TryGet<const ast::CExprInputs>(itemId))
					{
						for (Id pinId : inputs->pinIds)
						{
							if (IsTypeCompatible(ast, sourceType, GetPinType(ast, pinId)))
							{
								return true;
							}
						}
					}
				}
				else
				{
					// Output matches type?
					if (const auto* outputs = ast.TryGet<const ast::CExprOutputs>(itemId))
					{
						for (Id pinId : outputs->pinIds)
						{
							if (IsTypeCompatible(ast, GetPinType(ast, pinId), sourceType))
							{
								return true;
							}
						}
					}
				}
				break;
			case ContextItemKind::Literal:
				// Literals only produce a value, so they can only receive an input link
				return isExpr && context.type != Nodes::PinType::Output
				    && IsTypeCompatible(ast, itemId, sourceType);
			case ContextItemKind::Return: return false;
			case ContextItemKind::If:
				// if nodes have both an execution input and outputs, and only a bool
				// output fits the condition input
				return !isExpr || (isOutput && sourceType == ast.GetNativeTypes().boolId);
			case ContextItemKind::Variable:
				if (isExpr && !isOutput)
				{
					if (const auto* variable = ast.TryGet<const ast::CDeclVariable>(itemId))
					{
						return !IsNone(variable->typeId)
						    && IsTypeCompatible(ast, variable->typeId, sourceType);
					}
				}
				break;
			case ContextItemKind::Operator: return isExpr;
		}
		return false;
	}

	bool ContextItem(StringView name, Color background = Color::Transparent())
	{
		constexpr float rounding = 2.f;

		// Keep the click area tight to the item so the 1px spacing between items shows
		ImGuiSelectableFlags flags = ImGuiSelectableFlags_NoPadWithHalfSpacing;
		const bool hasBackground   = background.a > 0.f;
		if (hasBackground)
		{
			UI::PushHeaderColor(background);
		}

		UI::PushStyleVar(ImGuiStyleVar_ItemSpacing, v2{UI::GetStyle().ItemSpacing.x, 2.f});
		UI::PushStyleVar(ImGuiStyleVar_SelectableRounding, rounding);
		// Inset the text 4px from the background edges
		const float itemWidth  = UI::GetContentRegionAvail().x;
		const float textWidth  = UI::CalcTextSize(name).x;
		const float textAlignX = textWidth < itemWidth ? 4.f / (itemWidth - textWidth) : 0.f;
		UI::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, v2{textAlignX, 0.f});
		// A selected Selectable always paints its background
		const bool pressed = UI::Selectable(name, hasBackground, flags, v2{0.f, 16.f})
		                  || (UI::IsItemFocused() && UI::IsKeyPressed(ImGuiKey_Enter));
		UI::PopStyleVar(3);
		if (hasBackground)
		{
			UI::PopHeaderColor();
		}
		if (pressed)
		{
			UI::CloseCurrentPopup();
		}
		return pressed;
	}

	void DrawNodesContextMenu(ast::Tree& ast, Id typeId, TView<Id> nodeIds)
	{
		P_Check(!nodeIds.IsEmpty());
		const bool canEditBody = ast::HasFunctionBodies(ast, typeId);

		if (canEditBody && UI::MenuItem("Delete"))
		{
			ast::RemoveNodes(ast, nodeIds);
		}

		Id firstNodeId = nodeIds[0];
		if (nodeIds.Size() == 1 && ast.Has<ast::CDeclFunction>(firstNodeId))
		{
			if (canEditBody && UI::MenuItem("Add return node"))
			{
				Id newId = ast::AddReturn({ast, typeId});
				if (!IsNone(newId))
				{
					v2 position = ast.Get<CNodePosition>(firstNodeId).position;
					ast.Add<CNodePosition>(newId, position + v2{10.f, 0.f});

					ast::TryConnectStmt(ast, firstNodeId, newId);
				}
			}
		}
		TArray<Id> calls = FindIdsWith<ast::CExprCall>(ast, nodeIds);
		if (!calls.IsEmpty() && UI::MenuItem("Refresh"))
		{
			ast.AddN<CMdfd<ast::CExprCallId>>(calls);
		}
	}

	void DrawLinksContextMenu(ast::Tree& ast, Id typeId, TView<Id> linkIds)
	{
		P_Check(!linkIds.IsEmpty());
		const bool canEditBody = ast::HasFunctionBodies(ast, typeId);

		Id firstLinkId = linkIds[0];

		if (canEditBody && UI::MenuItem("Delete"))
		{
			ScopedChange(ast, linkIds);
			for (Id linkId : linkIds)
			{
				ast::DisconnectExpr(ast, ast::GetExprInputFromPin(ast, firstLinkId));
				ast::DisconnectStmtLink(ast, firstLinkId);
			}
		}
	}

	// Icon shown before a type/file category name
	static StringView GetFileIcon(ast::Tree& ast, Id typeId)
	{
		if (ast.Has<ast::CModule>(typeId))
		{
			return ICON_FA_TH_LARGE;
		}
		const StringView typeIcon = GetTypeIcon(ast, typeId);
		return typeIcon.empty() ? ICON_FA_FILE : typeIcon;
	}

	// Recursively draws a list of namespace ids: functions and variables become items,
	// modules and types become categories.
	static void DrawNamespaceChildren(ast::Tree& ast, Id typeId, const TArray<Id>& ids,
	    const ImGuiTextFilter& filter, const MenuContext& context)
	{
		for (Id childId : ids)
		{
			const auto* ns = ast.TryGet<const ast::CNamespace>(childId);
			if (!ns)
			{
				continue;
			}
			const StringView childName = ns->name.AsString();

			if (ast.Has<ast::CDeclFunction>(childId))
			{
				if (MatchesContext(
				        ast, context, ContextItemKind::Function, childName, filter, childId)
				    && ContextItem(childName))
				{
					Id newId = ast::AddCall({ast, typeId}, childId);
					// Populate the new call's pins right away so the dropped link can find
					// a matching pin to connect to
					ast::FunctionsSystem::SyncCallPinsFromFunction(ast);
					SetPositionAndConnect(ast, newId, context);
				}
			}
			else if (ast.Has<ast::CDeclVariable>(childId))
			{
				const auto* variable = ast.TryGet<const ast::CDeclVariable>(childId);
				if (MatchesContext(
				        ast, context, ContextItemKind::Variable, childName, filter, childId)
				    && ContextItem(
				        childName, GetTypeColor(ast, variable ? variable->typeId : ast::NoId)))
				{
					// Attach to the edited type, not the variable's owning type
					Id newId = ast::AddDeclarationReference({ast, typeId}, childId);
					SetPositionAndConnect(ast, newId, context);
				}
			}
			else
			{
				TArray<Id> categoryChildren;
				p::GetIdChildren(ast, childId, categoryChildren);
				categoryChildren.RemoveIf([&ast](Id id)
				{
					return ast.Has<ast::CModule>(id);    // Modules are drawn at root
				}, p::Shrink::No);
				if (categoryChildren.IsEmpty())
				{
					continue;
				}

				const StringView icon = GetFileIcon(ast, childId);
				const String label    = p::Format("{} {}", icon, childName);
				if (ContextTreeNode(label, filter))
				{
					DrawNamespaceChildren(ast, typeId, categoryChildren, filter, context);
					ContextTreePop(filter);
				}
			}
		}
	}

	// The conversions that can be applied to the dropped link. A cast node takes
	// the value in its input pin and produces the converted value in its output pin
	static bool MatchesConversionContext(
	    ast::Tree& ast, const MenuContext& context, const ast::Conversion& conversion)
	{
		if (!context.HasSource())
		{
			return true;
		}
		if (!IsExprPin(ast, context.sourcePin))
		{
			return false;    // Execution pins have no value to convert
		}
		const Id sourceType = GetPinType(ast, context.sourcePin);
		if (IsNone(sourceType))
		{
			return false;
		}
		if (context.type == Nodes::PinType::Output)
		{
			// The dropped value becomes the input of the conversion
			return IsTypeCompatible(ast, sourceType, conversion.fromType);
		}
		// The output of the conversion feeds the dropped pin
		return IsTypeCompatible(ast, conversion.toType, sourceType);
	}

	void DrawGraphContextMenu(ast::Tree& ast, Id typeId, MenuContext context)
	{
		static ImGuiTextFilter filter;
		const bool canEditBody = ast::HasFunctionBodies(ast, typeId);

		if (UI::IsWindowAppearing())
		{
			UI::SetKeyboardFocusHere();
			filter.Clear();
		}
		filter.Draw("##Filter");

		// Spawn at the release position when the menu came from a dropped link, so the
		// new node lands where the link was let go
		if (!context.HasSource())
		{
			context.location = UI::GetMousePosOnOpeningCurrentPopup();
		}

		if (!canEditBody)
		{
			return;
		}

		if (ContextTreeNode("Flow", filter))
		{
			if (MatchesContext(ast, context, ContextItemKind::Return, "Return", filter)
			    && ContextItem("Return"))
			{
				Id newId = ast::AddReturn({ast, typeId});
				SetPositionAndConnect(ast, newId, context);
			}
			if (MatchesContext(ast, context, ContextItemKind::If, "If", filter)
			    && ContextItem("If"))
			{
				Id newId = ast::AddIf({ast, typeId});
				SetPositionAndConnect(ast, newId, context);
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Operators", filter))
		{
			String name;
			// Unary operators
			for (auto type : GetEnumValues<ast::UnaryOperatorType>())
			{
				name.clear();
				StringView shortName = GetUnaryOperatorName(type);
				StringView longName  = GetUnaryOperatorLongName(type);
				p::FormatTo(name, "{}   ({})", shortName, longName);
				if (MatchesContext(ast, context, ContextItemKind::Operator, name, filter)
				    && ContextItem(name))
				{
					Id newId = ast::AddUnaryOperator({ast, typeId}, type);
					SetPositionAndConnect(ast, newId, context);
				}
			}
			// Binary operators
			for (auto type : GetEnumValues<ast::BinaryOperatorType>())
			{
				name.clear();
				StringView shortName = GetBinaryOperatorName(type);
				StringView longName  = GetBinaryOperatorLongName(type);
				p::FormatTo(name, "{}   ({})", shortName, longName);
				if (MatchesContext(ast, context, ContextItemKind::Operator, name, filter)
				    && ContextItem(name))
				{
					Id newId = ast::AddBinaryOperator({ast, typeId}, type);
					SetPositionAndConnect(ast, newId, context);
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Constructors", filter))
		{
			String makeStr{};
			TIdScope<ast::CDeclType, ast::CNamespace> typesScope{ast};
			for (Id literalTypeId : FindAllIdsWith<ast::CDeclType>(typesScope))
			{
				if (auto* ns = typesScope.TryGet<const ast::CNamespace>(literalTypeId))
				{
					makeStr.clear();
					p::FormatTo(makeStr, "Make {}", ns->name);
					const bool picked = MatchesContext(ast, context, ContextItemKind::Literal,
					                        makeStr, filter, literalTypeId)
					                 && ContextItem(makeStr, GetTypeColor(ast, literalTypeId));
					if (picked)
					{
						// Attach to the edited type, not the literal's own type
						Id newId = ast::AddLiteral({ast, typeId}, literalTypeId);
						SetPositionAndConnect(ast, newId, context);
					}
				}
			}
			ContextTreePop(filter);
		}

		if (ContextTreeNode("Casts", filter))
		{
			String name;
			for (const ast::Conversion& conversion : ast::ConversionSystem::GetConversions(ast))
			{
				name.clear();
				p::FormatTo(name, "{} {} {}", ast::GetName(ast, conversion.fromType).AsString(),
				    ICON_FA_ARROW_RIGHT, ast::GetName(ast, conversion.toType).AsString());
				if (!filter.PassFilter(name.data(), name.data() + name.size()))
				{
					continue;
				}
				if (!MatchesConversionContext(ast, context, conversion))
				{
					continue;
				}
				if (ContextItem(name, GetTypeColor(ast, conversion.toType)))
				{
					Id newId = ast::AddCast({ast, typeId}, conversion.fromType, conversion.toType);
					SetPositionAndConnect(ast, newId, context);
				}
			}
			ContextTreePop(filter);
		}

		// Functions and variables, grouped under their module and type namespaces
		DrawNamespaceChildren(
		    ast, typeId, p::FindAllIdsWith<ast::CModule, ast::CNamespace>(ast), filter, context);
	}

	void DrawContextMenu(ast::Tree& ast, Id typeId, MenuContext context)
	{
		// Cap the height so a long menu scrolls within the panel instead of overflowing it
		const float maxHeight = p::Min(UI::GetIO().DisplaySize.y * 0.3f, UI::GetWindowSize().y);
		UI::SetNextWindowSizeConstraints(ImVec2(0.f, 0.f), ImVec2(FLT_MAX, maxHeight));
		if (UI::BeginPopup("ContextMenu"))
		{
			if (!IsNone(context.hoveredNodeId))
			{
				if (Nodes::IsNodeSelected(context.hoveredNodeId))
				{
					DrawNodesContextMenu(ast, typeId, Nodes::GetSelectedNodes());
				}
				else
				{
					DrawNodesContextMenu(ast, typeId, context.hoveredNodeId);
				}
			}
			else if (!IsNone(context.hoveredLinkId))
			{
				TArray<Id> selectedLinkIds;
				if (Nodes::GetSelectedLinks(selectedLinkIds))
				{
					DrawLinksContextMenu(ast, typeId, selectedLinkIds);
				}
				else
				{
					DrawLinksContextMenu(ast, typeId, context.hoveredLinkId);
				}
			}
			else
			{
				DrawGraphContextMenu(ast, typeId, context);
			}
			UI::EndPopup();
		}
	}
}    // namespace rift::editor::Graph
