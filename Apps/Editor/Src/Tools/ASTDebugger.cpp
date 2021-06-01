// Copyright 2015-2021 Piperift - All rights reserved

#include "Tools/ASTDebugger.h"

#include <Lang/AST.h>
#include <Lang/CChildren.h>
#include <Lang/CParent.h>
#include <Lang/Identifiers/CIdentifier.h>
#include <UI/UI.h>


namespace Rift
{
	using namespace Memory;

	ASTDebugger::ASTDebugger() {}

	void ASTDebugger::Draw(AST::Tree& ast)
	{
		if (!open)
		{
			return;
		}

		ImGui::Begin("Abstract Syntax Tree", &open);
		{
			auto rootView = ast.MakeView<CChildren>(AST::TExclude<CParent>{});
			// auto identifierView = ast.MakeView<CIdentifier>();

			for (auto root : rootView)
			{
				DrawEntity(ast, root);
			}

			ast.EachOrphan([&ast](AST::Id ent) {
				DrawEntity(ast, ent);
			});
			ImGui::Separator();
		}
		ImGui::End();
	}

	void ASTDebugger::DrawEntity(AST::Tree& ast, AST::Id entity)
	{
		static String name;
		name.clear();
		if (CIdentifier* id = ast.GetComponentPtr<CIdentifier>(entity))
		{
			Strings::FormatTo(name, "{}  (id:{})", id->name, entity);
		}
		else
		{
			Strings::FormatTo(name, "(id:{})", entity);
		}

		const CChildren* children = ast.GetComponentPtr<CChildren>(entity);
		const bool hasChildren = children && !children->children.IsEmpty();
		if (ImGui::TreeNodeEx(name.c_str(), hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf))
		{
			if (hasChildren)
			{
				for (AST::Id child : children->children)
				{
					DrawEntity(ast, child);
				}
			}
			ImGui::TreePop();
		}
	}
}    // namespace Rift