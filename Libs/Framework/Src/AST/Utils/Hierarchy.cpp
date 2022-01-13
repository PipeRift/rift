// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Utils/Hierarchy.h"

#include "AST/Components/CChild.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "Misc/Checks.h"


namespace Rift::AST::Hierarchy
{
	void RemoveChildFromCParent(Tree& ast, Id parent, Id child)
	{
		if (CParent* cParent = GetCParent(ast, parent))
		{
			cParent->children.Remove(child);
			if (cParent->children.IsEmpty())
			{
				ast.Remove<CParent>(parent);
			}
		}
	}

	void AddChildren(Tree& ast, Id node, TSpan<const Id> children)
	{
		children.Each([&ast, node](Id child) {
			if (CChild* cChild = GetCChild(ast, child))
			{
				if (EnsureMsg(IsNone(cChild->parent),
				        "A node trying to be linked already has a parent. Consider using "
				        "TransferChildren()"))
				{
					cChild->parent = node;
				}
			}
			else
			{
				ast.Add<CChild>(child).parent = node;
			}
		});
		ast.GetOrAdd<CParent>(node).children.Append(children);
	}

	void AddChildrenAfter(Tree& ast, Id node, TSpan<Id> children, Id prevChild)
	{
		children.Each([&ast, node](Id child) {
			if (CChild* cChild = GetCChild(ast, child))
			{
				if (EnsureMsg(IsNone(cChild->parent),
				        "A node trying to be linked already has a parent. Consider using "
				        "TransferChildren()"))
				{
					cChild->parent = node;
				}
			}
			else
			{
				ast.Add<CChild>(child).parent = node;
			}
		});

		auto& childrenList  = ast.GetOrAdd<CParent>(node).children;
		const i32 prevIndex = childrenList.FindIndex(prevChild);
		childrenList.InsertRange(prevIndex, children);
	}

	void TransferChildren(Tree& ast, TSpan<Id> children, Id destination)
	{
		RemoveChildren(ast, children, true);
		AddChildren(ast, destination, children);
	}

	void TransferAllChildren(Tree& ast, Id origin, Id destination) {}

	void TransferChildren(Tree& ast, Id node, Id child) {}

	void RemoveChildren(Tree& ast, TSpan<Id> children, bool keepComponents)
	{
		TArray<Id> parents;
		parents.Reserve(children.Size());

		children.Each([&ast, &parents](Id child) {
			CChild* cChild = GetCChild(ast, child);
			if (cChild)
			{
				parents.Add(cChild->parent);
				cChild->parent = AST::NoId;
			}
		});

		if (!keepComponents)
		{
			children.Each([&ast](Id child) {
				ast.Remove<CChild>(child);
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

				if (CParent* cParent = GetCParent(ast, parent))
				{
					cParent->children.RemoveMany(children);
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

				if (CParent* cParent = GetCParent(ast, parent))
				{
					cParent->children.RemoveMany(children);
					if (cParent->children.IsEmpty())
					{
						ast.Remove<CParent>(parent);
					}
				}
			}
		}
	}

	void RemoveAllChildren(Tree& ast, TSpan<Id> parents, bool keepComponents)
	{
		if (keepComponents)
		{
			parents.Each([&ast](Id parent) {
				if (CParent* cParent = GetCParent(ast, parent))
				{
					for (Id child : cParent->children)
					{
						CChild* cChild = GetCChild(ast, child);
						Ensure(cChild);
						cChild->parent = AST::NoId;
					}
					cParent->children.Empty();
				}
			});
		}
		else
		{
			parents.Each([&ast](Id parent) {
				if (CParent* cParent = GetCParent(ast, parent))
				{
					for (Id child : cParent->children)
					{
						ast.Remove<CChild>(child);
					}
					ast.Remove<CParent>(parent);
				}
			});
		}
	}

	TArray<Id>* GetChildren(Tree& ast, Id node)
	{
		CParent* const parent = GetCParent(ast, node);
		return parent ? &parent->children : nullptr;
	}

	const TArray<Id>* GetChildren(const Tree& ast, Id node)
	{
		const CParent* const parent = GetCParent(ast, node);
		return parent ? &parent->children : nullptr;
	}

	void GetChildren(const Tree& ast, TSpan<const Id> nodes, TArray<Id>& outLinkedNodes)
	{
		nodes.Each([&ast, &outLinkedNodes](Id node) {
			if (const CParent* const parent = GetCParent(ast, node))
			{
				outLinkedNodes.Append(parent->children);
			}
		});
	}

	void GetChildrenDeep(
	    const Tree& ast, TSpan<const Id> roots, TArray<Id>& outLinkedNodes, u32 depth)
	{
		if (depth == 0)
		{
			TArray<AST::Id> currentLinked{};
			TArray<AST::Id> pendingInspection;
			pendingInspection.Append(roots);
			while (pendingInspection.Size() > 0)
			{
				GetChildren(ast, pendingInspection, currentLinked);
				outLinkedNodes.Append(currentLinked);
				pendingInspection = Move(currentLinked);
			}
		}
		else
		{
			TArray<AST::Id> currentLinked{};
			TArray<AST::Id> pendingInspection;
			pendingInspection.Append(roots);
			while (pendingInspection.Size() > 0 && depth > 0)
			{
				GetChildren(ast, pendingInspection, currentLinked);
				outLinkedNodes.Append(currentLinked);
				pendingInspection = Move(currentLinked);
				--depth;
			}
		}
	}

	Id GetParent(Tree& ast, Id node)
	{
		if (auto* child = GetCChild(ast, node))
		{
			return child->parent;
		}
		return AST::NoId;
	}

	TArray<Id> GetParents(const Tree& ast, TSpan<Id> nodes)
	{
		TArray<Id> parents;
		for (Id nodeId : nodes)
		{
			const auto* child = GetCChild(ast, nodeId);
			if (child && child->parent != AST::NoId)
			{
				parents.AddUnique(child->parent);
			}
		}
		return Move(parents);
	}

	AST::Id FindParent(AST::Tree& ast, AST::Id childId, const TFunction<bool(AST::Id)>& callback)
	{
		AST::Id parentId = GetParent(ast, childId);

		while (!IsNone(parentId))
		{
			if (callback(parentId))
			{
				return parentId;
			}
			parentId = GetParent(ast, childId);
		}
		return AST::NoId;
	}

	void Remove(Tree& ast, TSpan<Id> nodes)
	{
		RemoveChildren(ast, nodes, true);

		RemoveAllChildren(ast, nodes);
		ast.Destroy(nodes);
	}

	void RemoveDeep(Tree& ast, TSpan<Id> nodes)
	{
		RemoveChildren(ast, nodes, true);

		TArray<Id> allNodes;
		allNodes.Append(nodes);
		GetChildrenDeep(ast, nodes, allNodes);
		ast.Destroy(allNodes);
	}


	CChild* GetCChild(Tree& ast, Id node)
	{
		return reinterpret_cast<CChild*>(ast.GetChildView().TryGet<CChild>(node));
	}

	const CChild* GetCChild(const Tree& ast, Id node)
	{
		return reinterpret_cast<const CChild*>(ast.GetChildView().TryGet<CChild>(node));
	}

	CParent* GetCParent(Tree& ast, Id node)
	{
		return reinterpret_cast<CParent*>(ast.GetParentView().TryGet<CParent>(node));
	}

	const CParent* GetCParent(const Tree& ast, Id node)
	{
		return reinterpret_cast<const CParent*>(ast.GetParentView().TryGet<CParent>(node));
	}

	bool FixParentLinks(Tree& ast, TSpan<Id> parents)
	{
		bool fixed = false;
		for (Id parentId : parents)
		{
			if (const CParent* parent = GetCParent(ast, parentId))
			{
				for (Id childId : parent->children)
				{
					if (CChild* child = GetCChild(ast, childId))
					{
						if (child->parent != parentId)
						{
							child->parent = parentId;
							fixed         = true;
						}
					}
					else
					{
						ast.Add<CChild>(childId, parentId);
						fixed = true;
					}
				}
			}
		}
		return fixed;
	}

	bool ValidateParentLinks(const Tree& ast, TSpan<Id> parents)
	{
		for (Id parentId : parents)
		{
			if (const CParent* parent = GetCParent(ast, parentId))
			{
				for (Id childId : parent->children)
				{
					const CChild* child = GetCChild(ast, childId);
					if (!child || child->parent != parentId)
					{
						return false;
					}
				}
			}
		}
		return true;
	}
}    // namespace Rift::AST::Hierarchy
