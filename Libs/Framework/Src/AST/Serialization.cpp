// Copyright 2015-2021 Piperift - All rights reserved

#include "AST/Serialization.h"

#include "AST/Components/CChild.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CVariableDecl.h"
#include "AST/Components/Views/CGraphTransform.h"
#include "AST/Linkage.h"

#include <Reflection/TypeName.h>


namespace Rift
{
	template<typename T>
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
					ct.BeginObject();
					ct.Serialize(comp);
					ast.Emplace<T>(node, Move(comp));
					ct.Leave();
				}
			}
			ct.Leave();
		}
	}

	template<typename T>
	void WritePool(ASTWriteContext& ct, AST::Tree& ast, const TArray<AST::Id>& nodes)
	{
		auto view = ast.MakeView<T>();

		// FIX: yyjson doesn't seem to take into account stringview length when generating text
		// Temporarely fixed by caching component name keys
		ct.PushAddFlags(Serl::WriteFlags_CacheStringKeys);
		if (ct.EnterNext(GetTypeName<T>(false)))
		{
			String key;
			ct.BeginObject();
			for (i32 i = 0; i < nodes.Size(); ++i)
			{
				const AST::Id node = nodes[i];
				key.clear();
				Strings::FormatTo(key, "{}", i);

				if (view.Has(node))
				{
					if constexpr (std::is_empty_v<T>)
					{
						ct.Next(StringView{key}, T{});
					}
					else
					{
						ct.Next(StringView{key}, view.Get<T>(node));
					}
				}
			}
			ct.Leave();
		}
		ct.PopFlags();
	}

	void ASTReadContext::SerializeRoots(TArray<AST::Id>& roots)
	{
		TArray<AST::Id> parents = AST::GetLinkedParents(ast, roots);

		Next("count", nodeCount);
		ASTIds.Resize(nodeCount);
		// Create or assign root ids
		const i32 maxSize = Math::Min(roots.Size(), ASTIds.Size());
		for (i32 i = 0; i < maxSize; ++i)
		{
			const AST::Id root = roots[i];
			if (root != AST::NoId)
			{
				ASTIds[i] = root;
			}
			else
			{
				ASTIds[i] = ast.Create();
			}
		}

		// Create all non-root entities
		ast.Create(ASTIds.begin() + maxSize, ASTIds.end());

		// Next("roots", roots); // Not needed
		if (EnterNext("components"))
		{
			BeginObject();
			// TODO: Use reflection for this
			ReadPool<CChild>(*this, ast);
			ReadPool<CParent>(*this, ast);
			ReadPool<CIdentifier>(*this, ast);
			ReadPool<CStructDecl>(*this, ast);
			ReadPool<CClassDecl>(*this, ast);
			ReadPool<CVariableDecl>(*this, ast);
			ReadPool<CFunctionDecl>(*this, ast);
			ReadPool<CGraphTransform>(*this, ast);
			Leave();
		}

		AST::FixParentLinks(ast, parents);
	}

	void ASTWriteContext::SerializeRoots(const TArray<AST::Id>& roots)
	{
		TArray<AST::Id> treeEntities;
		RetrieveHierarchy(roots, treeEntities);
		nodeCount = treeEntities.Size();
		Next("count", nodeCount);

		ASTIdToIndexes.Reserve(nodeCount);
		for (u32 i = 0; i < nodeCount; ++i)
		{
			ASTIdToIndexes.Insert(treeEntities[i], i);
		}

		Next("roots", roots);
		if (EnterNext("components"))
		{
			BeginObject();
			// TODO: Use reflection for this
			WritePool<CChild>(*this, ast, treeEntities);
			WritePool<CParent>(*this, ast, treeEntities);
			WritePool<CIdentifier>(*this, ast, treeEntities);
			WritePool<CStructDecl>(*this, ast, treeEntities);
			WritePool<CClassDecl>(*this, ast, treeEntities);
			WritePool<CVariableDecl>(*this, ast, treeEntities);
			WritePool<CFunctionDecl>(*this, ast, treeEntities);
			WritePool<CGraphTransform>(*this, ast, treeEntities);
			Leave();
		}
	}

	void ASTWriteContext::RetrieveHierarchy(const TArray<AST::Id>& roots, TArray<AST::Id>& children)
	{
		children.Append(roots);
		if (includeChildren)
		{
			AST::GetLinkedDeep(ast, roots, children);
		}
	}
}    // namespace Rift
