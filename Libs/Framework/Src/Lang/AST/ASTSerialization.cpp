// Copyright 2015-2021 Piperift - All rights reserved

#include "Lang/AST/ASTSerialization.h"
#include "Lang/AST/ASTHelpers.h"
#include "Lang/CChildren.h"
#include "Lang/CParent.h"
#include "Lang/Declarations/CClassDecl.h"
#include "Lang/Declarations/CFunctionDecl.h"
#include "Lang/Declarations/CStructDecl.h"
#include "Lang/Declarations/CVariableDecl.h"
#include "Lang/Identifiers/CIdentifier.h"

#include <Reflection/TypeName.h>


namespace Rift
{
	template <typename T>
	void ReadPool(ASTReadContext& ct, AST::Tree& ast)
	{
		if (ct.EnterNext(GetTypeName<T>(false)))
		{
			const auto& nodes = ct.GetASTIds();

			String key;
			ct.BeginObject();
			for (i32 i = 0; i < nodes.Size(); ++i)
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
	void WritePool(ASTWriteContext& ct, const AST::Tree& ast, const TArray<AST::Id>& nodes)
	{
		if (ct.EnterNext(GetTypeName<T>()))
		{
			String key;
			ct.BeginObject();
			for (i32 i = 0; i < nodes.Size(); ++i)
			{
				const AST::Id node = nodes[i];
				key.clear();
				Strings::FormatTo(key, "{}", i);

				ct.Next(key, ast.GetComponent<T>(node));
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
			// TODO: Use reflection for this
			ReadPool<CParent>(*this, ast);
			ReadPool<CChildren>(*this, ast);
			ReadPool<CIdentifier>(*this, ast);
			ReadPool<CStructDecl>(*this, ast);
			ReadPool<CClassDecl>(*this, ast);
			ReadPool<CVariableDecl>(*this, ast);
			ReadPool<CFunctionDecl>(*this, ast);
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
			// TODO: Use reflection for this
			WritePool<CParent>(*this, ast, treeEntities);
			WritePool<CChildren>(*this, ast, treeEntities);
			WritePool<CIdentifier>(*this, ast, treeEntities);
			WritePool<CStructDecl>(*this, ast, treeEntities);
			WritePool<CClassDecl>(*this, ast, treeEntities);
			WritePool<CVariableDecl>(*this, ast, treeEntities);
			WritePool<CFunctionDecl>(*this, ast, treeEntities);
			Leave();
		}
	}

	void ASTWriteContext::RetrieveHierarchy(AST::Id root, TArray<AST::Id>& children)
	{
		children.Add(root);
		AST::GetLinkedDeep(ast, root, children);
	}
}    // namespace Rift
