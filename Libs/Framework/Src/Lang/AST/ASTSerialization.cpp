// Copyright 2015-2021 Piperift - All rights reserved

#include "Lang/AST/ASTSerialization.h"
#include "Lang/CChildren.h"
#include "Lang/CParent.h"
#include "Lang/Identifiers/CIdentifier.h"

#include <Reflection/TypeName.h>


namespace Rift
{
	template <typename T>
	void ReadPool(ASTReadContext& ct, AST::AbstractSyntaxTree& ast)
	{
		if (ct.EnterNext(GetTypeName<T>(false)))
		{
			const auto& nodes = ct.GetASTIds();

			String key;
			ct.BeginObject();
			for (u32 i = 0; i < nodes.Size(); ++i)
			{
				const AST::Id node = nodes[i];
				key.clear();
				Strings::FormatTo(key, "{}", i);

				if (ct.EnterNext(key))
				{
					T comp;
					ct.Serialize(comp);
					ast.AddComponent<T>(node, Move(comp));
					ct.Leave();
				}
			}
			ct.Leave();
		}
	}

	template <typename T>
	void WritePool(
	    ASTWriteContext& ct, const AST::AbstractSyntaxTree& ast, const TArray<AST::Id>& nodes)
	{
		if (ct.EnterNext(GetTypeName<T>()))
		{
			String key;
			ct.BeginObject();
			for (u32 i = 0; i < nodes.Size(); ++i)
			{
				const AST::Id node = nodes[i];
				key.clear();
				Strings::FormatTo(key, "{}", i);

				ct.Next(key, ast.GetComponent<const T>(node));
			}
			ct.Leave();
		}
	}

	void ASTReadContext::SerializeRoot(AST::Id& root)
	{
		Next("count", nodeCount);
		ASTIds.Reserve(nodeCount);
		for (u32 i = 0; i < nodeCount; ++i)
		{
			const AST::Id newNode = ast.Create();
			ASTIds.Add(newNode);
		}

		Next("root", root);
		if (EnterNext("components"))
		{
			BeginObject();
			ReadPool<CParent>(*this, ast);
			ReadPool<CChildren>(*this, ast);
			ReadPool<CIdentifier>(*this, ast);
			Leave();
		}
	}

	void ASTWriteContext::SerializeRoot(const AST::Id& root)
	{
		TArray<AST::Id> treeEntities;
		RetrieveHierarchy(root, treeEntities);
		nodeCount = treeEntities.Size();
		Next("count", nodeCount);

		ASTIdToIndexes.Reserve(nodeCount);
		for (u32 i = 0; i < nodeCount; ++i)
		{
			ASTIdToIndexes.Insert(treeEntities[i], i);
		}

		Next("root", root);
		if (EnterNext("components"))
		{
			BeginObject();
			// Iterate and serialize each component pool using treeEntities
			WritePool<CParent>(*this, ast, treeEntities);
			WritePool<CChildren>(*this, ast, treeEntities);
			WritePool<CIdentifier>(*this, ast, treeEntities);
			Leave();
		}
	}

	void ASTWriteContext::RetrieveHierarchy(AST::Id root, TArray<AST::Id>& children)
	{
		auto childrenView = ast.MakeView<const CChildren>();

		TArray<AST::Id> pendingInspection{root};
		TArray<AST::Id> newChildren{};
		while (pendingInspection.Size() > 0)
		{
			children.Append(pendingInspection);

			for (AST::Id parent : pendingInspection)
			{
				if (childrenView.Has(parent))
				{
					const auto& childrenComp = childrenView.Get<const CChildren>(parent);
					newChildren.Append(childrenComp.children);
				}
			}
			pendingInspection = Move(newChildren);
		}
	}
}    // namespace Rift
