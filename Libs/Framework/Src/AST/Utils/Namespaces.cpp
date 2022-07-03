// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/Namespaces.h"

#include "AST/Components/CNamespace.h"
#include "AST/Utils/Hierarchy.h"


namespace rift::AST
{
	bool GetRelativeNamespace(
	    TAccessRef<CNamespace, CChild> access, Id id, p::String& ns, Id relativeParentId)
	{
		if (IsNone(relativeParentId))
		{
			relativeParentId = AST::Hierarchy::GetParent(access, id);
		}

		ns.clear();

		Id currentId = AST::Hierarchy::GetParent(access, id);
		while (!IsNone(currentId))
		{
			currentId = AST::Hierarchy::GetParent(access, id);
		}
	}

	void GetFullNamespace(TAccessRef<CNamespace, CChild> access, Id id, p::String& ns)
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
			if (auto* ident = access.TryGet<const CNamespace>(id))
			{
				ns.append(ident->name.ToString());
			}
			else
			{
				ns.append(Name::None().ToString());
			}

			if (i != 0)
			{
				ns.append(".");
			}
		}
	}

	bool GetName(TAccessRef<CNamespace, CChild> access, Id id, p::String& name,
	    bool includeNamespace, bool relativeNamespace)
	{
		GetFullNamespace(access, id, name);
	}
}    // namespace rift::AST
