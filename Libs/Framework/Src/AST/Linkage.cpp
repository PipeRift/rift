// Copyright 2015-2020 Piperift - All rights reserved
#include "AST/Linkage.h"

#include "AST/Components/CChildren.h"
#include "AST/Components/CClassDecl.h"
#include "AST/Components/CFunctionDecl.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStructDecl.h"
#include "AST/Components/CVariableDecl.h"
#include "Misc/Checks.h"


namespace Rift::AST
{
	void RemoveChildFromCChildren(Tree& ast, Id parent, Id child)
	{
		if (CChildren* children = GetCChildren(ast, parent))
		{
			children->children.Remove(child);
			if (children->children.IsEmpty())
			{
				ast.RemoveComponent<CChildren>(parent);
			}
		}
	}


	Id CreateClass(Tree& ast, Name name)
	{
		Id id = ast.Create();
		ast.AddComponent<CIdentifier>(id, name);
		ast.AddComponents<CStructDecl, CChildren>(id);
		return id;
	}

	Id CreateStruct(Tree& ast, Name name)
	{
		Id id = ast.Create();
		ast.AddComponent<CIdentifier>(id, name);
		ast.AddComponents<CStructDecl, CChildren>(id);
		return id;
	}

	Id CreateVariable(Tree& ast, Name name)
	{
		Id id = ast.Create();
		ast.AddComponent<CIdentifier>(id, name);
		ast.AddComponents<CVariableDecl, CChildren>(id);
		return id;
	}

	Id CreateFunction(Tree& ast, Name name)
	{
		Id id = ast.Create();
		ast.AddComponent<CIdentifier>(id, name);
		ast.AddComponents<CFunctionDecl, CChildren>(id);
		return id;
	}


	void Link(Tree& ast, Id node, TArrayView<Id> children)
	{
		children.Each([&ast, node](Id child) {
			if (CParent* parentComp = GetCParent(ast, child))
			{
				if (EnsureMsg(!IsValid(parentComp->parent),
				        "A node trying to be linked already has a parent. Consider using "
				        "TransferLinks()"))
				{
					parentComp->parent = node;
				}
			}
			else
			{
				ast.AddComponent<CParent>(child).parent = node;
			}
		});
		ast.GetOrAddComponent<CChildren>(node).children.Append(children);
	}

	void TransferLinks(Tree& ast, TArrayView<Id> children, Id destination)
	{
		Unlink(ast, children, true);
		Link(ast, destination, children);
	}

	void TransferAllLinks(Tree& ast, Id origin, Id destination) {}

	void TransferLinks(Tree& ast, Id node, Id child) {}

	void Unlink(Tree& ast, TArrayView<Id> children, bool keepComponents)
	{
		TArray<Id> parents;
		parents.Reserve(children.Size());

		children.Each([&ast, &parents](Id child) {
			CParent* parentComp = GetCParent(ast, child);
			if (parentComp)
			{
				parents.Add(parentComp->parent);
				parentComp->parent = AST::NoId;
			}
		});

		if (!keepComponents)
		{
			children.Each([&ast](Id child) {
				ast.RemoveComponent<CParent>(child);
			});
		}

		// Sort to iterate avoiding duplicated parents
		parents.Sort();
		Id lastParent = AST::NoId;

		if (keepComponents)
		{
			for (Id parent : parents)
			{
				if (parent == lastParent)
				{
					continue;
				}
				lastParent = parent;

				if (CChildren* childrenComp = GetCChildren(ast, parent))
				{
					childrenComp->children.RemoveMany(children);
				}
			}
		}
		else
		{
			for (Id parent : parents)
			{
				if (parent == lastParent)
				{
					continue;
				}
				lastParent = parent;

				if (CChildren* childrenComp = GetCChildren(ast, parent))
				{
					childrenComp->children.RemoveMany(children);
					if (childrenComp->children.IsEmpty())
					{
						ast.RemoveComponent<CChildren>(parent);
					}
				}
			}
		}
	}

	void UnlinkAllChildren(Tree& ast, TArrayView<Id> parents, bool keepComponents)
	{
		if (keepComponents)
		{
			parents.Each([&ast](Id parent) {
				if (CChildren* childrenComp = GetCChildren(ast, parent))
				{
					for (Id child : childrenComp->children)
					{
						CParent* parentComp = GetCParent(ast, child);
						Ensure(parentComp);
						parentComp->parent = AST::NoId;
					}
					childrenComp->children.Empty();
				}
			});
		}
		else
		{
			parents.Each([&ast](Id parent) {
				if (CChildren* childrenComp = GetCChildren(ast, parent))
				{
					for (Id child : childrenComp->children)
					{
						ast.RemoveComponent<CParent>(child);
					}
					ast.RemoveComponent<CChildren>(parent);
				}
			});
		}
	}

	TArray<Id>* GetLinked(Tree& ast, Id node)
	{
		CChildren* const parent = GetCChildren(ast, node);
		return parent ? &parent->children : nullptr;
	}

	void GetLinked(Tree& ast, TArrayView<Id> nodes, TArray<Id>& outLinkedNodes)
	{
		nodes.Each([&ast, &outLinkedNodes](Id node) {
			if (CChildren* const parent = GetCChildren(ast, node))
			{
				outLinkedNodes.Append(parent->children);
			}
		});
	}

	void GetLinkedDeep(const Tree& ast, TArrayView<Id> roots, TArray<Id>& outLinkedNodes)
	{
		TArray<AST::Id> pendingInspection;
		pendingInspection.Append(roots);
		TArray<AST::Id> currentLinked{};
		while (pendingInspection.Size() > 0)
		{
			for (AST::Id parent : pendingInspection)
			{
				if (const CChildren* childrenComp = GetCChildren(ast, parent))
				{
					currentLinked.Append(childrenComp->children);
				}
			}
			outLinkedNodes.Append(currentLinked);
			pendingInspection = Move(currentLinked);
		}
	}

	void Remove(Tree& ast, TArrayView<Id> nodes)
	{
		Unlink(ast, nodes, true);

		UnlinkAllChildren(ast, nodes);
		ast.Destroy(nodes.begin(), nodes.end());
	}

	void RemoveDeep(Tree& ast, TArrayView<Id> nodes)
	{
		Unlink(ast, nodes, true);

		TArray<Id> allNodes;
		allNodes.Append(nodes);
		GetLinkedDeep(ast, nodes, allNodes);
		ast.Destroy(allNodes.begin(), allNodes.end());
	}


	CParent* GetCParent(Tree& ast, Id node)
	{
		return ast.parentView.TryGet<CParent>(node);
	}

	const CParent* GetCParent(const Tree& ast, Id node)
	{
		return ast.parentView.TryGet<CParent>(node);
	}

	CChildren* GetCChildren(Tree& ast, Id node)
	{
		return ast.childrenView.TryGet<CChildren>(node);
	}

	const CChildren* GetCChildren(const Tree& ast, Id node)
	{
		return ast.childrenView.TryGet<CChildren>(node);
	}
}    // namespace Rift::AST
