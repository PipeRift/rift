// Copyright 2015-2021 Piperift - All rights reserved

#include "AST/Serialization.h"

#include "AST/Components/CBoolLiteral.h"
#include "AST/Components/CCallExpr.h"
#include "AST/Components/CChild.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CCompoundStmt.h"
#include "AST/Components/CFloatLiteral.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStringLiteral.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CVariableDecl.h"
#include "AST/Components/Views/CGraphTransform.h"
#include "AST/Utils/Hierarchy.h"

#include <Reflection/TypeName.h>


namespace Rift::AST
{
	template<typename T>
	void ReadPool(ReadContext& ct, Tree& ast)
	{
		if (ct.EnterNext(GetTypeName<T>(false)))
		{
			const auto& nodes = ct.GetIds();

			String key;
			ct.BeginObject();
			for (i32 i = 0; i < nodes.Size(); ++i)
			{
				const Id node = nodes[i];
				key.clear();
				Strings::FormatTo(key, "{}", i);

				if (ct.EnterNext(key))
				{
					ct.BeginObject();
					if constexpr (!IsEmpty<T>())
					{
						T& comp = ast.GetOrAdd<T>(node);
						ct.Serialize(comp);
					}
					else if (!ast.Has<T>(node))
					{
						ast.Add<T>(node);
					}
					ct.Leave();
				}
			}
			ct.Leave();
		}
	}

	template<typename T>
	void WritePool(WriteContext& ct, Tree& ast, const TArray<Id>& nodes)
	{
		auto view = ast.Query<T>();

		// FIX: yyjson doesn't seem to take into account stringview length when generating text
		// Temporarely fixed by caching component name keys
		ct.PushAddFlags(Serl::WriteFlags_CacheStringKeys);
		if (ct.EnterNext(GetTypeName<T>(false)))
		{
			String key;
			ct.BeginObject();
			for (i32 i = 0; i < nodes.Size(); ++i)
			{
				const Id node = nodes[i];
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
						ct.Next(StringView{key}, view.template Get<T>(node));
					}
				}
			}
			ct.Leave();
		}
		ct.PopFlags();
	}

	void ReadContext::SerializeRoots(TArray<Id>& roots)
	{
		TArray<Id> parents = Hierarchy::GetParents(ast, roots);

		Next("count", nodeCount);
		ids.Resize(i32(nodeCount));
		// Create or assign root ids
		const i32 maxSize = Math::Min(roots.Size(), ids.Size());
		for (i32 i = 0; i < maxSize; ++i)
		{
			const Id root = roots[i];
			if (root != NoId)
			{
				ids[i] = root;
			}
			else
			{
				ids[i] = ast.Create();
			}
		}

		// Create all non-root entities
		ast.Create({ids.Data() + maxSize, ids.Data() + ids.Size()});

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
			ReadPool<CCallExpr>(*this, ast);
			ReadPool<CCompoundStmt>(*this, ast);
			ReadPool<CBoolLiteral>(*this, ast);
			ReadPool<CFloatLiteral>(*this, ast);
			ReadPool<CStringLiteral>(*this, ast);
			ReadPool<CGraphTransform>(*this, ast);
			Leave();
		}

		Hierarchy::FixParentLinks(ast, parents);
	}

	void WriteContext::SerializeRoots(const TArray<Id>& roots)
	{
		TArray<Id> treeEntities;
		RetrieveHierarchy(roots, treeEntities);
		nodeCount = treeEntities.Size();
		Next("count", nodeCount);

		idToIndexes.Reserve(nodeCount);
		for (i32 i = 0; i < i32(nodeCount); ++i)
		{
			idToIndexes.Insert(treeEntities[i], i);
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
			WritePool<CCallExpr>(*this, ast, treeEntities);
			WritePool<CCompoundStmt>(*this, ast, treeEntities);
			WritePool<CBoolLiteral>(*this, ast, treeEntities);
			WritePool<CFloatLiteral>(*this, ast, treeEntities);
			WritePool<CStringLiteral>(*this, ast, treeEntities);
			WritePool<CGraphTransform>(*this, ast, treeEntities);
			Leave();
		}
	}

	void WriteContext::RetrieveHierarchy(const TArray<Id>& roots, TArray<Id>& children)
	{
		children.Append(roots);
		if (includeChildren)
		{
			Hierarchy::GetLinkedDeep(ast, roots, children);
		}
	}
}    // namespace Rift::AST
