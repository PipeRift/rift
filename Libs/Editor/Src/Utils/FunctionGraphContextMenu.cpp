// Copyright 2015-2022 Piperift - All rights reserved

#include "Utils/FunctionGraphContextMenu.h"

#include "Utils/FunctionGraph.h"
#include "Utils/FunctionUtils.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CType.h>
#include <AST/Components/Views/CGraphTransform.h>
#include <AST/Statics/STypes.h>
#include <AST/Utils/Statements.h>


namespace Rift::Graph
{

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

	void DrawNodeContextMenu(AST::Tree& ast, AST::Id typeId, TSpan<AST::Id> nodeIds)
	{
		Check(!nodeIds.IsEmpty());

		AST::Id firstNodeId = nodeIds[0];

		if (nodeIds.Size() == 1 && ast.Has<CDeclFunction>(firstNodeId))
		{
			if (UI::MenuItem("Add return node"))
			{
				AST::Id newId = Functions::AddReturn({ast, typeId});
				if (!IsNone(newId))
				{
					v2 position = ast.Get<CGraphTransform>(firstNodeId).position;
					ast.Add<CGraphTransform>(newId, position + v2{10.f, 0.f});

					AST::Statements::TryConnect(ast, firstNodeId, newId);
				}
			}
			UI::Separator();
		}
		if (UI::MenuItem("Delete"))
		{
			Functions::RemoveNodes(ast, nodeIds);
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

		if (filter.IsActive() || UI::TreeNode("Flow"))
		{
			if (ContextItem("Return", filter))
			{
				AST::Id newId = Functions::AddReturn({ast, typeId});
				if (!IsNone(newId))
				{
					ast.Add<CGraphTransform>(newId, gridPos);
				}
			}
			if (ContextItem("If", filter))
			{
				AST::Id newId = Functions::AddIf({ast, typeId});
				if (!IsNone(newId))
				{
					ast.Add<CGraphTransform>(newId, gridPos);
				}
			}

			if (!filter.IsActive())
			{
				UI::TreePop();
			}
		}

		if (filter.IsActive() || UI::TreeNode("Constructors"))
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
						AST::Id newId = Functions::AddLiteral({ast, typeId}, it.second);
						if (!IsNone(newId))
						{
							ast.Add<CGraphTransform>(newId, gridPos);
						}
					}
				}
			}

			if (!filter.IsActive())
			{
				UI::TreePop();
			}
		}

		if (filter.IsActive() || UI::TreeNode("Functions"))
		{
			auto functions   = ast.Filter<CDeclFunction>();
			auto identifiers = ast.Filter<CIdentifier>();
			for (AST::Id functionId : functions)
			{
				if (auto* iden = identifiers.TryGet<CIdentifier>(functionId))
				{
					const String& name = iden->name.ToString();
					if (ContextItem(name, filter))
					{
						AST::Id newId = Functions::AddCall({ast, typeId}, functionId);
						if (!IsNone(newId))
						{
							ast.Add<CGraphTransform>(newId, gridPos);
						}
					}
				}
			}

			if (!filter.IsActive())
			{
				ImGui::TreePop();
			}
		}

		if (filter.IsActive() || UI::TreeNode("Variables"))
		{
			auto variables   = ast.Filter<CDeclVariable>();
			auto identifiers = ast.Filter<CIdentifier>();
			for (AST::Id variableId : variables)
			{
				if (auto* iden = identifiers.TryGet<CIdentifier>(variableId))
				{
					const String& name = iden->name.ToString();

					if (ContextItem(name, filter))
					{
						AST::Id newId =
						    Functions::AddDeclarationReference({ast, typeId}, variableId);
						if (!IsNone(newId))
						{
							ast.Add<CGraphTransform>(newId, gridPos);
						}
					}
				}
			}

			if (!filter.IsActive())
			{
				UI::TreePop();
			}
		}

		if (filter.IsActive() || UI::TreeNode("Operators"))
		{
			static String name;
			// Unary operators
			for (auto type : Refl::GetEnumValues<UnaryOperatorType>())
			{
				StringView shortName = Functions::GetUnaryOperatorName(type);
				StringView longName  = Functions::GetUnaryOperatorLongName(type);
				name.clear();
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (ContextItem(name, filter))
				{
					AST::Id newId = Functions::AddUnaryOperator({ast, typeId}, type);
					if (!IsNone(newId))
					{
						ast.Add<CGraphTransform>(newId, gridPos);
					}
				}
			}
			// Binary operators
			for (auto type : Refl::GetEnumValues<BinaryOperatorType>())
			{
				StringView shortName = Functions::GetBinaryOperatorName(type);
				StringView longName  = Functions::GetBinaryOperatorLongName(type);
				name.clear();
				Strings::FormatTo(name, "{}   ({})", shortName, longName);
				if (ContextItem(name, filter))
				{
					AST::Id newId = Functions::AddBinaryOperator({ast, typeId}, type);
					if (!IsNone(newId))
					{
						ast.Add<CGraphTransform>(newId, gridPos);
					}
				}
			}

			if (!filter.IsActive())
			{
				UI::TreePop();
			}
		}
	}

	void DrawContextMenu(AST::Tree& ast, AST::Id typeId, AST::Id hoveredNodeId)
	{
		if (ImGui::BeginPopup("GraphContextMenu"))
		{
			if (IsNone(hoveredNodeId))
			{
				DrawGraphContextMenu(ast, typeId);
			}
			else if (Nodes::IsNodeSelected(hoveredNodeId))
			{
				DrawNodeContextMenu(ast, typeId, Nodes::GetSelectedNodes());
			}
			else
			{
				DrawNodeContextMenu(ast, typeId, hoveredNodeId);
			}
			ImGui::EndPopup();
		}
	}
}    // namespace Rift::Graph
