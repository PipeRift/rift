// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Serialization.h"

#include "AST/Components/CBoolLiteral.h"
#include "AST/Components/CCallExpr.h"
#include "AST/Components/CChild.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CExpressionInput.h"
#include "AST/Components/CExpressionOutputs.h"
#include "AST/Components/CFloatLiteral.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParameterDecl.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CReturnExpr.h"
#include "AST/Components/CStatementInput.h"
#include "AST/Components/CStatementOutputs.h"
#include "AST/Components/CStringLiteral.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CVariableDecl.h"
#include "AST/Components/Tags/CNotSerialized.h"
#include "AST/Components/Views/CGraphTransform.h"
#include "AST/Filtering.h"
#include "AST/Utils/Hierarchy.h"

#include <Reflection/TypeName.h>


namespace Rift::AST
{
	template<typename T>
	void ReadPool(ReadContext& ct, TAccessRef<TWrite<T>> access)
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
						T& comp = access.GetOrAdd<T>(node);
						ct.Serialize(comp);
					}
					else if (!access.Has<T>(node))
					{
						access.Add<T>(node);
					}
					ct.Leave();
				}
			}
			ct.Leave();
		}
	}

	template<typename T>
	void WritePool(WriteContext& ct, TAccessRef<T> access, const TArray<Id>& nodes)
	{
		TArray<Id> componentIds = GetIf<T>(access, nodes);
		if (componentIds.IsEmpty())
		{
			return;
		}

		// FIX: yyjson doesn't seem to take into account stringview length when generating text
		// Temporarely fixed by caching component name keys
		ct.PushAddFlags(Serl::WriteFlags_CacheStringKeys);
		if (ct.EnterNext(GetTypeName<T>(false)))
		{
			String key;
			ct.BeginObject();
			for (i32 i = 0; i < componentIds.Size(); ++i)
			{
				key.clear();
				Strings::FormatTo(key, "{}", i);

				if constexpr (std::is_empty_v<T>)
				{
					ct.Next(StringView{key}, T{});
				}
				else
				{
					ct.Next(StringView{key}, access.template Get<const T>(componentIds[i]));
				}
			}
			ct.Leave();
		}
		ct.PopFlags();
	}

	void ReadContext::SerializeEntities(TArray<Id>& entities)
	{
		TArray<Id> parents;
		Hierarchy::GetParents(ast, entities, parents);

		Next("count", nodeCount);
		ids.Resize(i32(nodeCount));
		// Create or assign root ids
		const i32 maxSize = Math::Min(entities.Size(), ids.Size());
		for (i32 i = 0; i < maxSize; ++i)
		{
			const Id entity = entities[i];
			if (entity != NoId)
			{
				ids[i] = entity;
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
			ReadPool<CParameterDecl>(*this, ast);
			ReadPool<CCallExpr>(*this, ast);
			ReadPool<CStatementOutputs>(*this, ast);
			ReadPool<CStatementInput>(*this, ast);
			ReadPool<CExpressionOutputs>(*this, ast);
			ReadPool<CExpressionInput>(*this, ast);
			ReadPool<CBoolLiteral>(*this, ast);
			ReadPool<CFloatLiteral>(*this, ast);
			ReadPool<CStringLiteral>(*this, ast);
			ReadPool<CGraphTransform>(*this, ast);
			ReadPool<CReturnExpr>(*this, ast);
			Leave();
		}

		Hierarchy::FixParentLinks(ast, parents);
	}

	void WriteContext::SerializeEntities(const TArray<Id>& entities, bool includeChildren)
	{
		TArray<Id> treeEntities;
		if (includeChildren)
		{
			RetrieveHierarchy(entities, treeEntities);
		}
		else
		{
			treeEntities = entities;
		}
		nodeCount = treeEntities.Size();
		Next("count", nodeCount);

		idToIndexes.Reserve(nodeCount);
		for (i32 i = 0; i < i32(nodeCount); ++i)
		{
			idToIndexes.Insert(treeEntities[i], i);
		}

		Next("roots", entities);
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
			WritePool<CParameterDecl>(*this, ast, treeEntities);
			WritePool<CCallExpr>(*this, ast, treeEntities);
			WritePool<CStatementOutputs>(*this, ast, treeEntities);
			WritePool<CStatementInput>(*this, ast, treeEntities);
			WritePool<CExpressionOutputs>(*this, ast, treeEntities);
			WritePool<CExpressionInput>(*this, ast, treeEntities);
			WritePool<CBoolLiteral>(*this, ast, treeEntities);
			WritePool<CFloatLiteral>(*this, ast, treeEntities);
			WritePool<CStringLiteral>(*this, ast, treeEntities);
			WritePool<CGraphTransform>(*this, ast, treeEntities);
			WritePool<CReturnExpr>(*this, ast, treeEntities);
			Leave();
		}
	}

	void WriteContext::RetrieveHierarchy(const TArray<Id>& roots, TArray<Id>& children)
	{
		children.Append(roots);
		if (includeChildren)
		{
			TArray<AST::Id> currentLinked{};
			TArray<AST::Id> pendingInspection;
			pendingInspection.Append(roots);
			while (pendingInspection.Size() > 0)
			{
				RemoveIgnoredEntities(pendingInspection);
				Hierarchy::GetChildren(ast, pendingInspection, currentLinked);
				children.Append(currentLinked);
				pendingInspection = Move(currentLinked);
			}
		}
	}

	void WriteContext::RemoveIgnoredEntities(TArray<Id>& entities)
	{
		auto notSerialized = ast.Filter<CNotSerialized>();
		for (i32 i = 0; i < entities.Size(); ++i)
		{
			if (notSerialized.Has(entities[i]))
			{
				entities.RemoveAtSwapUnsafe(i);
				--i;
			}
		}
		entities.Shrink();
	}
}    // namespace Rift::AST
