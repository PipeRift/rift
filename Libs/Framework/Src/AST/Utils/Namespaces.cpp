// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/Namespaces.h"

#include "AST/Components/CNamespace.h"
#include "AST/Id.h"
#include "Pipe/Core/StringView.h"

#include <Pipe/ECS/Filtering.h>
#include <Pipe/ECS/Utils/Hierarchy.h>
#include <Pipe/Math/Math.h>


namespace rift::AST
{
	Namespace::Namespace(StringView value)
	{
		i32 size          = 0;
		const TChar* last = value.data() + value.size();
		const TChar* curr = value.data();

		if (curr != last && *curr == '@')
			++curr;

		const TChar* scopeStart = curr;
		while (curr != last && size < scopeCount)
		{
			if (*curr == '.')
			{
				scopes[size] = Name{
				    StringView{scopeStart, curr}
                };
				scopeStart = curr + 1;
				++size;
			}
			++curr;
		}

		if (scopeStart < curr)    // Add last
		{
			scopes[size] = Name{
			    StringView{scopeStart, curr}
            };
		}
	}

	bool Namespace::Equals(const Namespace& other) const
	{
		for (i32 i = 0; i < scopeCount; ++i)
		{
			const Name scope      = scopes[i];
			const Name otherScope = other.scopes[i];
			if (scope != otherScope)
			{
				return false;
			}
			else if (scope.IsNone() && otherScope.IsNone())
			{
				return true;
			}
		}
		return true;
	}

	bool Namespace::IsEmpty() const
	{
		return First().IsNone();
	}

	i32 Namespace::Size() const
	{
		i32 size = 0;
		while (size < scopeCount && !scopes[size].IsNone())
		{
			++size;
		}
		return size;
	}

	p::String Namespace::ToString(LocalNamespace isLocal) const
	{
		p::String ns;
		if (!isLocal)
		{
			ns.append("@");
			const Name firstScope = scopes[0];
			if (!firstScope.IsNone())
			{
				ns.append(firstScope.ToString());
				ns.append(".");
			}
		}
		for (i32 i = 1; i < scopeCount; ++i)
		{
			const Name scope = scopes[i];
			if (scope.IsNone())
			{
				break;
			}
			ns.append(scope.ToString());
			ns.append(".");
		}

		if (!ns.empty())    // Remove last dot
		{
			ns.pop_back();
		}
		return Move(ns);
	}

	void Namespace::Read(p::Reader& ct)
	{
		u32 size = 0;
		ct.BeginArray(size);
		size = p::math::Min(size, u32(Namespace::scopeCount));
		for (u32 i = 0; i < size; ++i)
		{
			ct.Next(scopes[i]);
		}
	}

	void Namespace::Write(p::Writer& ct) const
	{
		u32 size = Size();
		ct.BeginArray(size);
		for (u32 i = 0; i < size; ++i)
		{
			ct.Next(scopes[i]);
		}
	}


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
			id = p::ecs::GetParent(access, id);
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
			return GetNamespace(access, p::ecs::GetParent(access, id));
		}
		return {};
	}

	Id FindIdFromNamespace(TAccessRef<CNamespace, CChild, CParent> access, const Namespace& ns,
	    const TArray<Id>* rootIds)
	{
		TArray<Id> localRoots;
		if (!rootIds)
		{
			localRoots = ecs::ListAll<CNamespace>(access);
			ecs::ExcludeIf<CChild>(access, localRoots);
			rootIds = &localRoots;
		}

		const TArray<Id>* scopeIds = rootIds;
		Id foundScopeId            = NoId;
		Name scopeName;
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
				scopeIds = p::ecs::GetChildren(access, foundScopeId);
				++depth;
			}
			else
			{
				scopeIds = nullptr;    // Nothing more to iterate
			}
		}
		return foundScopeId;
	}

	Name GetName(TAccessRef<CNamespace> access, Id id)
	{
		auto* ns = access.TryGet<const CNamespace>(id);
		return ns ? ns->name : Name::None();
	}
	Name GetNameUnsafe(TAccessRef<CNamespace> access, Id id)
	{
		return access.Get<const CNamespace>(id).name;
	}

	p::String GetFullName(
	    TAccessRef<CNamespace, CChild, CModule> access, Id id, LocalNamespace localNamespace)
	{
		return GetNamespace(access, id).ToString(localNamespace);
	}

}    // namespace rift::AST
