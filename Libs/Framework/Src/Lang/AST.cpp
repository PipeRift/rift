// Copyright 2015-2020 Piperift - All rights reserved

#include "Lang/AST.h"


namespace Rift::AST
{
	Id Tree::Create()
	{
		return registry.create();
	}
	Id Tree::Create(const Id hint)
	{
		return registry.create(Move(hint));
	}

	void Tree::Destroy(const Id node)
	{
		registry.destroy(node);
	}

	void Tree::Destroy(const Id node, const VersionType version)
	{
		registry.destroy(node, version);
	}

	void Tree::RemoveChildFromCChildren(Id parent, Id child)
	{
		if (CChildren* children = GetComponentPtr<CChildren>(parent))
		{
			children->children.Remove(child);
			if (children->children.IsEmpty())
			{
				RemoveComponent<CChildren>(parent);
			}
		}
	}
}    // namespace Rift::AST
