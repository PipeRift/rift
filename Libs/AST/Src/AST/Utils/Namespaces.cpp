// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Utils/Namespaces.h"

#include "AST/Components/CNamespace.h"
#include "AST/Id.h"
#include "Pipe/Core/StringView.h"

#include <PipeECS.h>
#include <PipeMath.h>


namespace rift::ast
{
	Namespace GetNamespace(TAccessRef<CNamespace, CChild, CModule> access, Id id)
	{
		Namespace ns;
		TArray<Id> idChain;
		idChain.Reserve(Namespace::scopeCount);

		while (!IsNone(id))
		{
			idChain.Add(id);
			if (access.Has<CModule>(id))
			{
				break;
			}
			id = p::GetIdParent(access, id);
		}

		i32 i, scopeIndex = 0;
		for (i = idChain.Size() - 1; i >= 0 && scopeIndex < Namespace::scopeCount; --i)
		{
			ns.scopes[scopeIndex] = GetName(access, idChain[i]);
			++scopeIndex;
		}
		CheckMsg(i < 0, "Not enough scopes to cover this namespace");
		return ns;
	}

	Namespace GetParentNamespace(TAccessRef<CNamespace, CChild, CModule> access, Id id)
	{
		if (!IsNone(id))
		{
			return GetNamespace(access, p::GetIdParent(access, id));
		}
		return {};
	}

	Id FindIdFromNamespace(TAccessRef<CNamespace, CChild, CParent> access, const Namespace& ns,
	    const TArray<Id>* rootIds)
	{
		TArray<Id> localRoots;
		if (!rootIds)
		{
			localRoots = FindAllIdsWith<CNamespace>(access);
			ExcludeIdsWith<CChild>(access, localRoots);
			rootIds = &localRoots;
		}

		const TArray<Id>* scopeIds = rootIds;
		Id foundScopeId            = NoId;
		Tag scopeName;
		i32 depth = 0;
		while (scopeIds && depth < Namespace::scopeCount)
		{
			scopeName = ns[depth];
			if (scopeName.IsNone())
			{
				break;
			}

			foundScopeId = NoId;
			for (Id id : *scopeIds)
			{
				auto* rootName = access.TryGet<const CNamespace>(id);
				if (rootName && rootName->name == scopeName)
				{
					foundScopeId = id;
					break;
				}
			}

			if (!IsNone(foundScopeId))
			{
				// Found matching name, check next scope
				scopeIds = p::GetIdChildren(access, foundScopeId);
				++depth;
			}
			else
			{
				scopeIds = nullptr;    // Nothing more to iterate
			}
		}
		return foundScopeId;
	}

	Tag GetName(TAccessRef<CNamespace> access, Id id)
	{
		auto* ns = access.TryGet<const CNamespace>(id);
		return ns ? ns->name : Tag::None();
	}
	Tag GetNameUnsafe(TAccessRef<CNamespace> access, Id id)
	{
		return access.Get<const CNamespace>(id).name;
	}

	p::String GetFullName(
	    TAccessRef<CNamespace, CChild, CModule> access, Id id, bool localNamespace)
	{
		return GetNamespace(access, id).ToString(localNamespace);
	}

}    // namespace rift::ast
