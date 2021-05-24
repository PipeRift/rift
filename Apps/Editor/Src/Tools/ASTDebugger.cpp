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

	void ASTDebugger::Draw(AST::AbstractSyntaxTree& ast)
	{
		if (!open)
		{
			return;
		}

		ImGui::Begin("Abstract Syntax Tree", &open);
		{
			auto childrenView = ast.MakeView<CChildren>();
			auto parentView   = ast.MakeView<CParent>();
			// auto identifierView = ast.MakeView<CIdentifier>();

			for (auto root : childrenView)
			{
				if (parentView.Has(root))
				{
					continue;
				}

				DrawEntity(ast, root);
			}
			ImGui::Separator();
		}
		ImGui::End();
	}

	void ASTDebugger::DrawEntity(AST::AbstractSyntaxTree& ast, AST::Id entity)
	{
		CIdentifier* id     = ast.GetComponentPtr<CIdentifier>(entity);
		CChildren* children = ast.GetComponentPtr<CChildren>(entity);

		static const String none{""};
		const String& name = id ? id->name.ToString() : none;

		const bool hasChildren = children && !children->children.IsEmpty();
		if (ImGui::TreeNodeEx(name.c_str(), hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf))
		{
			for (auto child : children->children)
			{
				DrawEntity(ast, child);
			}
			ImGui::TreePop();
		}
	}
}    // namespace Rift
