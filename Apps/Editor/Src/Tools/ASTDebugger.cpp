// Copyright 2015-2021 Piperift - All rights reserved

#include "Tools/ASTDebugger.h"

#include <ECS.h>
#include <Lang/CChildren.h>
#include <Lang/CParent.h>
#include <Lang/Identifiers/Identifier.h>
#include <UI/UI.h>


namespace Rift
{
	using namespace Memory;

	ASTDebugger::ASTDebugger() {}

	void ASTDebugger::Draw(AST& ast)
	{
		if (!open)
		{
			return;
		}

		ImGui::Begin("Abstract Syntax Tree", &open);
		{
			auto& registry = ast.GetRegistry();

			auto rootEntities = registry.MakeView<CIdentifier>();
			for (auto root : rootEntities)
			{
				CIdentifier& id    = rootEntities.Get<CIdentifier>(root);
				const String& name = id.name.ToString();
				if (ImGui::TreeNode(name.c_str()))
				{
					ImGui::TreePop();
				}
			}
			ImGui::Separator();
		}
		ImGui::End();
	}
}    // namespace Rift
