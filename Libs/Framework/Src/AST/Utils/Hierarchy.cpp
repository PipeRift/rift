// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/Hierarchy.h"

#include "AST/Components/CChild.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "Misc/Checks.h"


namespace Rift::AST::Hierarchy
{
	void RemoveChildFromCParent(TAccessRef<TWrite<CParent>> access, Id parent, Id child)
	{
		if (auto* cParent = access.TryGet<CParent>(parent))
		{
			cParent->children.Remove(child);
			if (cParent->children.IsEmpty())
			{
				access.Remove<CParent>(parent);
			}
		}
	}

	void AddChildren(
	    TAccessRef<TWrite<CChild>, TWrite<CParent>> access, Id node, TSpan<const Id> children)
	{
		children.Each([&access, node](Id child) {
			if (CChild* cChild = access.TryGet<CChild>(child))
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
				access.Add<CChild>(child).parent = node;
			}
		});
		access.GetOrAdd<CParent>(node).children.Append(children);
	}

	void AddChildrenAfter(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, Id node,
	    TSpan<Id> children, Id prevChild)
	{
		children.Each([&access, node](Id child) {
			if (CChild* cChild = access.TryGet<CChild>(child))
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
				access.Add<CChild>(child).parent = node;
			}
		});

		auto& childrenList  = access.GetOrAdd<CParent>(node).children;
		const i32 prevIndex = childrenList.FindIndex(prevChild);
		childrenList.InsertRange(prevIndex, children);
	}

	void TransferChildren(
	    TAccessRef<TWrite<CChild>, TWrite<CParent>> access, TSpan<Id> children, Id destination)
	{
		RemoveChildren(access, children, true);
		AddChildren(access, destination, children);
	}

	void RemoveChildren(
	    TAccessRef<TWrite<CParent>, TWrite<CChild>> access, TSpan<Id> children, bool keepComponents)
	{
		TArray<Id> parents;
		parents.Reserve(children.Size());

		children.Each([&access, &parents](Id child) {
			if (auto* cChild = access.TryGet<CChild>(child))
			{
				parents.Add(cChild->parent);
				cChild->parent = AST::NoId;
			}
		});

		if (!keepComponents)
		{
			children.Each([&access](Id child) {
				access.Remove<CChild>(child);
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

				if (auto* cParent = access.TryGet<CParent>(parent))
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

				if (auto* cParent = access.TryGet<CParent>(parent))
				{
					cParent->children.RemoveMany(children);
					if (cParent->children.IsEmpty())
					{
						access.Remove<CParent>(parent);
					}
				}
			}
		}
	}

	void RemoveAllChildren(
	    TAccessRef<TWrite<CParent>, TWrite<CChild>> access, TSpan<Id> parents, bool keepComponents)
	{
		if (keepComponents)
		{
			parents.Each([&access](Id parent) {
				if (auto* cParent = access.TryGet<CParent>(parent))
				{
					for (Id child : cParent->children)
					{
						access.Get<CChild>(parent).parent = AST::NoId;
					}
					cParent->children.Empty();
				}
			});
		}
		else
		{
			parents.Each([&access](Id parent) {
				if (auto* cParent = access.TryGet<CParent>(parent))
				{
					for (Id child : cParent->children)
					{
						access.Remove<CChild>(child);
					}
					access.Remove<CParent>(parent);
				}
			});
		}
	}

	TArray<Id>* GetMutChildren(TAccessRef<TWrite<CParent>> access, Id node)
	{
		auto* cParent = access.TryGet<CParent>(node);
		return cParent ? &cParent->children : nullptr;
	}

	const TArray<Id>* GetChildren(TAccessRef<CParent> access, Id node)
	{
		auto* cParent = access.TryGet<const CParent>(node);
		return cParent ? &cParent->children : nullptr;
	}

	void GetChildren(TAccessRef<CParent> access, TSpan<const Id> nodes, TArray<Id>& outLinkedNodes)
	{
		nodes.Each([&access, &outLinkedNodes](Id node) {
			if (const auto* cParent = access.TryGet<const CParent>(node))
			{
				outLinkedNodes.Append(cParent->children);
			}
		});
	}

	void GetChildrenDeep(
	    TAccessRef<CParent> access, TSpan<const Id> roots, TArray<Id>& outLinkedNodes, u32 depth)
	{
		if (depth == 0)
		{
			TArray<AST::Id> currentLinked{};
			TArray<AST::Id> pendingInspection;
			pendingInspection.Append(roots);
			while (pendingInspection.Size() > 0)
			{
				GetChildren(access, pendingInspection, currentLinked);
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
				GetChildren(access, pendingInspection, currentLinked);
				outLinkedNodes.Append(currentLinked);
				pendingInspection = Move(currentLinked);
				--depth;
			}
		}
	}

	Id GetParent(TAccessRef<CChild> access, Id node)
	{
		if (const auto* cChild = access.TryGet<const CChild>(node))
		{
			return cChild->parent;
		}
		return AST::NoId;
	}

	void GetParents(TAccessRef<CChild> access, TSpan<Id> children, TArray<Id>& outParents)
	{
		outParents.Empty(false);
		for (Id childId : children)
		{
			const auto* child = access.TryGet<const CChild>(childId);
			if (child && !IsNone(child->parent))
			{
				outParents.AddUnique(child->parent);
			}
		}
	}

	AST::Id FindParent(
	    TAccessRef<CChild> access, AST::Id childId, const TFunction<bool(AST::Id)>& callback)
	{
		while (!IsNone(childId))
		{
			childId = GetParent(access, childId);
			if (callback(childId))
			{
				return childId;
			}
		}
		return AST::NoId;
	}
	void FindParents(TAccessRef<CChild> access, TSpan<Id> childrenIds, TArray<Id>& outParents,
	    const TFunction<bool(AST::Id)>& callback)
	{
		outParents.Empty(false);

		TArray<Id> children{childrenIds.begin(), childrenIds.Size()};
		TArray<Id> parents;

		while (children.Size() > 0)
		{
			GetParents(access, children, parents);
			for (i32 i = 0; i < parents.Size(); ++i)
			{
				const Id parentId = parents[i];
				if (callback(parentId))
				{
					outParents.Add(parentId);
					parents.RemoveAtSwap(i, false);
				}
			}
			Swap(children, parents);
			parents.Empty(false);
		}
	}

	void Remove(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, TSpan<Id> nodes)
	{
		RemoveChildren(access, nodes, true);

		RemoveAllChildren(access, nodes);
		access.GetAST().Destroy(nodes);
	}

	void RemoveDeep(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, TSpan<Id> nodes)
	{
		RemoveChildren(access, nodes, true);

		TArray<Id> allNodes;
		allNodes.Append(nodes);
		GetChildrenDeep(access, nodes, allNodes);
		access.GetAST().Destroy(allNodes);
	}


	bool FixParentLinks(TAccessRef<TWrite<CChild>, CParent> access, TSpan<Id> parents)
	{
		bool fixed = false;
		for (Id parentId : parents)
		{
			if (const CParent* parent = access.TryGet<const CParent>(parentId))
			{
				for (Id childId : parent->children)
				{
					if (CChild* child = access.TryGet<CChild>(childId))
					{
						if (child->parent != parentId)
						{
							child->parent = parentId;
							fixed         = true;
						}
					}
					else
					{
						access.Add<CChild>(childId, parentId);
						fixed = true;
					}
				}
			}
		}
		return fixed;
	}

	bool ValidateParentLinks(TAccessRef<CChild, CParent> access, TSpan<Id> parents)
	{
		for (Id parentId : parents)
		{
			if (const auto* parent = access.TryGet<const CParent>(parentId))
			{
				for (Id childId : parent->children)
				{
					const auto* child = access.TryGet<const CChild>(childId);
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
