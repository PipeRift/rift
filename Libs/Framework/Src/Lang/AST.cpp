// Copyright 2015-2020 Piperift - All rights reserved

#include "Lang/AST.h"

#include "Lang/CChildren.h"
#include "Lang/CParent.h"
#include "Lang/Declarations/CClassDecl.h"
#include "Lang/Declarations/CFunctionDecl.h"
#include "Lang/Declarations/CStructDecl.h"
#include "Lang/Declarations/CVariableDecl.h"
#include "Lang/Identifiers/CIdentifier.h"


namespace Rift::AST
{
	Id AbstractSyntaxTree::CreateClass(Name name)
	{
		Id id = Create();
		AddComponent<CIdentifier>(id, name);
		AddComponent<CClassDecl>(id);
		AddComponent<CChildren>(id);
		return id;
	}

	Id AbstractSyntaxTree::CreateStruct(Name name)
	{
		Id id = Create();
		AddComponent<CIdentifier>(id, name);
		AddComponent<CStructDecl>(id);
		AddComponent<CChildren>(id);
		return id;
	}

	Id AbstractSyntaxTree::CreateVariable(Name name)
	{
		Id id = Create();
		AddComponent<CIdentifier>(id, name);
		AddComponent<CVariableDecl>(id);
		AddComponent<CChildren>(id);
		return id;
	}

	Id AbstractSyntaxTree::CreateFunction(Name name)
	{
		Id id = Create();
		AddComponent<CIdentifier>(id, name);
		AddComponent<CFunctionDecl>(id);
		AddComponent<CChildren>(id);
		return id;
	}

	Id AbstractSyntaxTree::Create()
	{
		return registry.create();
	}
	Id AbstractSyntaxTree::Create(const Id hint)
	{
		return registry.create(Move(hint));
	}

	void AbstractSyntaxTree::Destroy(const Id node)
	{
		registry.destroy(node);
	}

	void AbstractSyntaxTree::Destroy(const Id node, const VersionType version)
	{
		registry.destroy(node, version);
	}

	void AbstractSyntaxTree::RemoveChildFromCChildren(Id parent, Id child)
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

	void AbstractSyntaxTree::AddChild(Id parent, Id child)
	{
		if (CParent* oldParentComp = GetComponentPtr<CParent>(child))
		{
			// Remove old linkage
			RemoveChildFromCChildren(oldParentComp->parent, child);
			oldParentComp->parent = parent;
		}
		else
		{
			AddComponent<CParent>(child).parent = parent;
		}
		GetOrAddComponent<CChildren>(parent).children.Add(child);
	}
	void AbstractSyntaxTree::RemoveChild(Id parent, Id child)
	{
		RemoveChildFromCChildren(parent, child);
		RemoveComponent<CParent>(child);
	}
}    // namespace Rift::AST
