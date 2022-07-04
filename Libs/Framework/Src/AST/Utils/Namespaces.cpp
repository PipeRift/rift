// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/Namespaces.h"

#include "AST/Components/CNamespace.h"
#include "AST/Utils/Hierarchy.h"


namespace rift::AST
{
	void GetNamespace(TAccessRef<CNamespace, CChild> access, Id id, p::String& ns)
	{
		ns.clear();
		TArray<Id> idChain;
		Id currentId = AST::Hierarchy::GetParent(access, id);
		while (!IsNone(currentId))
		{
			idChain.Add(currentId);
			currentId = AST::Hierarchy::GetParent(access, currentId);
		}

		for (i32 i = idChain.Size() - 1; i >= 0; --i)
		{
			const Id id = idChain[i];
			ns.append(GetName(access, id).ToString());
			ns.append(".");
		}

		if (idChain.Size() > 0)
		{
			ns.pop_back();
		}
	}

	void GetLocalNamespace(TAccessRef<CNamespace, CChild, CModule> access, Id id, p::String& ns)
	{
		ns.clear();
		TArray<Id> idChain;
		Id currentId = AST::Hierarchy::GetParent(access, id);
		while (!IsNone(currentId) && !access.Has<CModule>(currentId))
		{
			idChain.Add(currentId);
			currentId = AST::Hierarchy::GetParent(access, currentId);
		}

		for (i32 i = idChain.Size() - 1; i >= 0; --i)
		{
			const Id id = idChain[i];
			ns.append(GetName(access, id).ToString());
			ns.append(".");
		}

		if (idChain.Size() > 0)
		{
			ns.pop_back();
		}
	}

	Name GetName(TAccessRef<CNamespace, CChild> access, Id id)
	{
		if (auto* ns = access.TryGet<const CNamespace>(id))
		{
			return ns->name;
		}
		return Name::None();
	}

	p::String&& GetFullName(TAccessRef<CNamespace, CChild> access, Id id, bool relativeNamespace)
	{
		p::String name;
		GetNamespace(access, id, name);
		if (!name.empty())
		{
			name.push_back('.');
		}
		name.append(GetName(access, id).ToString());
		return Move(name);
	}
}    // namespace rift::AST
