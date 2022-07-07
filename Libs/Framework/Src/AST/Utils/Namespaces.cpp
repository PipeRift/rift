// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/Namespaces.h"

#include "AST/Components/CNamespace.h"
#include "AST/Utils/Hierarchy.h"


namespace rift::AST
{
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
		return GetFirstScope().IsNone();
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
		for (i32 i = bool(isLocal) ? 1 : 0; i < scopeCount; ++i)
		{
			Name scope = scopes[i];
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

	Namespace GetNamespace(TAccessRef<CNamespace, CChild, CModule> access, Id id)
	{
		Namespace ns;
		TArray<Id> idChain;
		idChain.Reserve(Namespace::scopeCount);

		Id currentId = AST::Hierarchy::GetParent(access, id);
		while (!IsNone(currentId))
		{
			idChain.Add(currentId);
			if (access.Has<CModule>(currentId))
			{
				break;
			}
			currentId = AST::Hierarchy::GetParent(access, currentId);
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

	Name GetName(TAccessRef<CNamespace> access, Id id)
	{
		auto* ns = access.TryGet<const CNamespace>(id);
		return ns ? ns->name : Name::None();
	}
	Name GetNameChecked(TAccessRef<CNamespace> access, Id id)
	{
		return access.Get<const CNamespace>(id).name;
	}

	p::String GetFullName(
	    TAccessRef<CNamespace, CChild, CModule> access, Id id, LocalNamespace localNamespace)
	{
		Namespace ns   = GetNamespace(access, id);
		p::String name = ns.ToString(localNamespace);
		if (!name.empty())
		{
			name.push_back('.');
		}
		name.append(GetName(access, id).ToString());
		return Move(name);
	}
	p::String GetFullNameChecked(
	    TAccessRef<CNamespace, CChild, CModule> access, Id id, LocalNamespace localNamespace)
	{
		Namespace ns   = GetNamespace(access, id);
		p::String name = ns.ToString(localNamespace);
		if (!name.empty())
		{
			name.push_back('.');
		}
		name.append(GetNameChecked(access, id).ToString());
		return Move(name);
	}
}    // namespace rift::AST
