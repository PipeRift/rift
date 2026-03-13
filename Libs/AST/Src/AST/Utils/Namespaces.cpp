// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "AST/Utils/Namespaces.h"

#include "AST/Components/CNamespace.h"
#include "AST/Id.h"
#include "Pipe/Core/StringView.h"

#include <PipeECS.h>
#include <PipeMath.h>


namespace rift::ast
{
	Namespace GetNamespace(p::TIdScopeRef<CNamespace, CChild, CModule> scope, Id id)
	{
		Namespace ns;
		p::TArray<Id> idChain;
		idChain.Reserve(Namespace::scopeCount);

		while (!IsNone(id))
		{
			idChain.Add(id);
			if (scope.Has<CModule>(id))
			{
				break;
			}
			id = p::GetIdParent(scope, id);
		}

		p::i32 i, scopeIndex = 0;
		for (i = idChain.Size() - 1; i >= 0 && scopeIndex < Namespace::scopeCount; --i)
		{
			ns.scopes[scopeIndex] = GetName(scope, idChain[i]);
			++scopeIndex;
		}
		P_CheckMsg(i < 0, "Not enough scopes to cover this namespace");
		return ns;
	}

	Namespace GetParentNamespace(p::TIdScopeRef<CNamespace, CChild, CModule> scope, Id id)
	{
		if (!IsNone(id))
		{
			return GetNamespace(scope, p::GetIdParent(scope, id));
		}
		return {};
	}

	Id FindIdFromNamespace(p::TIdScopeRef<CNamespace, CChild, CParent> scope, const Namespace& ns,
	    const p::TArray<Id>* rootIds)
	{
		p::TArray<Id> localRoots;
		if (!rootIds)
		{
			localRoots = p::FindAllIdsWith<CNamespace>(scope);
			p::ExcludeIdsWith<CChild>(scope, localRoots);
			rootIds = &localRoots;
		}

		const p::TArray<Id>* scopeIds = rootIds;
		Id foundScopeId               = NoId;
		p::Tag scopeName;
		p::i32 depth = 0;
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
				auto* rootName = scope.TryGet<const CNamespace>(id);
				if (rootName && rootName->name == scopeName)
				{
					foundScopeId = id;
					break;
				}
			}

			if (!IsNone(foundScopeId))
			{
				// Found matching name, check next scope
				scopeIds = p::GetIdChildren(scope, foundScopeId);
				++depth;
			}
			else
			{
				scopeIds = nullptr;    // Nothing more to iterate
			}
		}
		return foundScopeId;
	}

	p::Tag GetName(p::TIdScopeRef<CNamespace> scope, Id id)
	{
		auto* ns = scope.TryGet<const CNamespace>(id);
		return ns ? ns->name : p::Tag::None();
	}
	p::Tag GetNameUnsafe(p::TIdScopeRef<CNamespace> scope, Id id)
	{
		return scope.Get<const CNamespace>(id).name;
	}

	p::String GetFullName(p::TIdScopeRef<CNamespace, CChild, CModule> scope, Id id,
	    bool localNamespace, char separator)
	{
		return GetNamespace(scope, id).ToString(localNamespace, separator);
	}

}    // namespace rift::ast
