// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Serialization.h"

#include "AST/Components/CChild.h"
#include "AST/Components/CDeclClass.h"
#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CDeclStruct.h"
#include "AST/Components/CDeclVariable.h"
#include "AST/Components/CExprCall.h"
#include "AST/Components/CExprInput.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprReturn.h"
#include "AST/Components/CExprType.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CLiteralBool.h"
#include "AST/Components/CLiteralFloat.h"
#include "AST/Components/CLiteralString.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
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
						T& comp = access.template GetOrAdd<T>(node);
						ct.Serialize(comp);
					}
					else if (!access.template Has<T>(node))
					{
						access.template Add<T>(node);
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
		TArray<TPair<i32, Id>> componentIds;

		if (auto* pool = access.template GetPool<const T>())
		{
			componentIds.ReserveMore(Math::Min(i32(pool->Size()), nodes.Size()));
			for (i32 i = 0; i < nodes.Size(); ++i)
			{
				const Id id = nodes[i];
				if (pool->Has(id))
				{
					componentIds.Add({i, id});
				}
			}
		}

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
			for (auto id : componentIds)
			{
				key.clear();
				Strings::FormatTo(key, "{}", id.first);

				if constexpr (std::is_empty_v<T>)
				{
					ct.Next(StringView{key}, T{});
				}
				else
				{
					ct.Next(StringView{key}, access.template Get<const T>(id.second));
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
			ReadPool<CDeclClass>(*this, ast);
			ReadPool<CDeclFunction>(*this, ast);
			ReadPool<CDeclStruct>(*this, ast);
			ReadPool<CDeclVariable>(*this, ast);
			ReadPool<CExprCall>(*this, ast);
			ReadPool<CExprInput>(*this, ast);
			ReadPool<CExprOutputs>(*this, ast);
			ReadPool<CExprReturn>(*this, ast);
			ReadPool<CExprType>(*this, ast);
			ReadPool<CIdentifier>(*this, ast);
			ReadPool<CGraphTransform>(*this, ast);
			ReadPool<CParent>(*this, ast);
			ReadPool<CLiteralBool>(*this, ast);
			ReadPool<CLiteralFloat>(*this, ast);
			ReadPool<CLiteralString>(*this, ast);
			ReadPool<CStmtOutputs>(*this, ast);
			ReadPool<CStmtInput>(*this, ast);
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
			WritePool<CDeclStruct>(*this, ast, treeEntities);
			WritePool<CDeclClass>(*this, ast, treeEntities);
			WritePool<CDeclVariable>(*this, ast, treeEntities);
			WritePool<CDeclFunction>(*this, ast, treeEntities);
			WritePool<CExprCall>(*this, ast, treeEntities);
			WritePool<CExprInput>(*this, ast, treeEntities);
			WritePool<CExprOutputs>(*this, ast, treeEntities);
			WritePool<CExprReturn>(*this, ast, treeEntities);
			WritePool<CExprType>(*this, ast, treeEntities);
			WritePool<CGraphTransform>(*this, ast, treeEntities);
			WritePool<CIdentifier>(*this, ast, treeEntities);
			WritePool<CParent>(*this, ast, treeEntities);
			WritePool<CLiteralBool>(*this, ast, treeEntities);
			WritePool<CLiteralFloat>(*this, ast, treeEntities);
			WritePool<CLiteralString>(*this, ast, treeEntities);
			WritePool<CStmtOutputs>(*this, ast, treeEntities);
			WritePool<CStmtInput>(*this, ast, treeEntities);
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
