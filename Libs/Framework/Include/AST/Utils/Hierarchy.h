// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CChild.h"
#include "AST/Components/CParent.h"
#include "AST/Id.h"

#include <Pipe/Core/Span.h>
#include <Pipe/ECS/Access.h>


namespace rift::AST::Hierarchy
{
	// Link a list of nodes at the end of the parent children list
	void AddChildren(
	    TAccessRef<TWrite<CChild>, TWrite<CParent>> access, Id node, TSpan<const Id> children);
	// Link a list of nodes after prevChild in the list of children nodes
	void AddChildrenAfter(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, Id node,
	    TSpan<Id> children, Id prevChild);
	void TransferChildren(
	    TAccessRef<TWrite<CChild>, TWrite<CParent>> access, TSpan<Id> children, Id destination);
	// TODO: void TransferAllChildren(Tree& ast, Id origin, Id destination);
	void RemoveChildren(TAccessRef<TWrite<CParent>, TWrite<CChild>> access, TSpan<Id> children,
	    bool keepComponents);
	void RemoveAllChildren(TAccessRef<TWrite<CParent>, TWrite<CChild>> access, TSpan<Id> parents,
	    bool keepComponents = false);

	TArray<Id>* GetMutChildren(TAccessRef<TWrite<CParent>> access, Id node);
	const TArray<Id>* GetChildren(TAccessRef<CParent> access, Id node);
	void GetChildren(TAccessRef<CParent> access, TSpan<const Id> nodes, TArray<Id>& outLinkedNodes);
	/**
	 * Finds all nodes connected recursively.
	 */
	void GetChildrenDeep(TAccessRef<CParent> access, TSpan<const Id> roots,
	    TArray<Id>& outLinkedNodes, u32 depth = 0);
	Id GetParent(TAccessRef<CChild> access, Id node);
	void GetParents(TAccessRef<CChild> access, TSpan<const Id> children, TArray<Id>& outParents);
	void GetAllParents(TAccessRef<CChild> access, Id node, TArray<Id>& outParents);
	void GetAllParents(
	    TAccessRef<CChild> access, TSpan<const Id> childrenIds, TArray<Id>& outParents);

	/**
	 * Find a parent id matching a delegate
	 */
	AST::Id FindParent(
	    TAccessRef<CChild> access, AST::Id child, const TFunction<bool(AST::Id)>& callback);
	void FindParents(TAccessRef<CChild> access, TSpan<const Id> children, TArray<Id>& outParents,
	    const TFunction<bool(AST::Id)>& callback);

	// void Copy(Tree& ast, t TArray<Id>& nodes, TArray<Id>& outNewNodes);
	// void CopyDeep(Tree& ast, const TArray<Id>& rootNodes, TArray<Id>& outNewRootNodes);
	// void CopyAndTransferAllChildrenDeep(Tree& ast, Id root, Id otherRoot);

	void Remove(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, TSpan<Id> nodes);
	void RemoveDeep(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, TSpan<Id> nodes);

	/**
	 * Iterates children nodes making sure child->parent links are correct or fixed
	 * Only first depth links are affected
	 * Complexity: O(N)
	 * @parents: where to look for children to fix up
	 * @return true if an incorrect link was found and fixed
	 */
	bool FixParentLinks(TAccessRef<TWrite<CChild>, CParent> access, TSpan<Id> parents);

	/**
	 * Iterates children nodes looking for invalid child->parent links
	 * Only first depth links are affected
	 * Complexity: O(1) <-> O(N) (First invalid link makes an early out)
	 * @parents: where to look for children
	 * @return true if an incorrect link was found
	 */
	bool ValidateParentLinks(TAccessRef<CChild, CParent> access, TSpan<Id> parents);
}    // namespace rift::AST::Hierarchy
