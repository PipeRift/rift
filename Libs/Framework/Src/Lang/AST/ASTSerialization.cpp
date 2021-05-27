// Copyright 2015-2021 Piperift - All rights reserved

#include "Lang/AST/ASTSerialization.h"
#include "Lang/CChildren.h"


namespace Rift
{
	void ASTReadContext::SerializeRoot(AST::Id& root)
	{
		Next("count", nodeCount);
		indexToASTIds.Reserve(nodeCount);
		for (u32 i = 0; i < nodeCount; ++i)
		{
			// Create entities
			indexToASTIds.Insert(i, ast.Create());
		}

		Next("root", root);
		if (EnterNext("components"))
		{
			Leave();
		}
	}

	void ASTWriteContext::SerializeRoot(const AST::Id& root)
	{
		TArray<AST::Id> treeEntities;
		RetrieveHierarchy(root, treeEntities);
		nodeCount = treeEntities.Size();
		Next("count", nodeCount);

		for (u32 i = 0; i < nodeCount; ++i)
		{
			ASTIdToIndexes.Insert(treeEntities[i], i);
		}

		Next("root", root);
		if (EnterNext("components"))
		{
			// Iterate and serialize each component pool using treeEntities
			Leave();
		}
	}

	void ASTWriteContext::RetrieveHierarchy(AST::Id root, TArray<AST::Id>& children)
	{
		auto childrenView = ast.MakeView<CChildren>();

		TArray<AST::Id> pendingInspection{root};
		TArray<AST::Id> newChildren{};
		while (pendingInspection.Size() > 0)
		{
			children.Append(pendingInspection);

			for (AST::Id parent : pendingInspection)
			{
				if (childrenView.Has(parent))
				{
					const auto& childrenComp = childrenView.Get<CChildren>(parent);
					newChildren.Append(childrenComp.children);
				}
			}
			pendingInspection = Move(newChildren);
		}
	}
}    // namespace Rift
