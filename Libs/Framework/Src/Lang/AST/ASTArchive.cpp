// Copyright 2015-2021 Piperift - All rights reserved

#include "Lang/AST/ASTArchive.h"

#include "Lang/CChildren.h"


namespace Rift
{
	/*void ASTArchive::SerializeRoot(AST::Id& root)
	{
		if (IsLoading())
		{
			Serialize("nodeCount", nodeCount);

			for (u32 i = 0; i < nodeCount; ++i) {}
		}
		else    // Saving
		{
			TArray<AST::Id> treeEntities;
			RetrieveHierarchy(root, treeEntities);

			nodeCount = treeEntities.Size();
			Serialize("nodeCount", nodeCount);

			for (u32 i = 0; i < nodeCount; ++i)
			{
				ASTToDataIds.Insert(treeEntities[i], i);
			}

			BeginObject("components");
			EndObject();

			// Iterate and serialize each component pool using treeEntities
		}
	}

	void ASTArchive::RetrieveHierarchy(AST::Id root, TArray<AST::Id>& children)
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
	}*/

	/*void ASTArchive::Serialize(StringView name, AST::Id& val)
	{
	    if (IsLoading())
	    {
	        u32 dataId;
	        Serialize(name, dataId);
	        val = DataToASTIds[dataId];
	    }
	    else
	    {
	        u32 dataId = ASTToDataIds[val];
	        Serialize(name, dataId);
	    }
	}*/
}    // namespace Rift
