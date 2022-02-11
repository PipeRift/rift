// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Access.h"

#include <Containers/Span.h>


namespace Rift
{
	struct CParent;
	struct CChild;
}    // namespace Rift


namespace Rift::AST::Hierarchy
{
	// Link a list of nodes at the end of the parent children list
	void AddChildren(TAccessRef<CChild, CParent> access, Id node, TSpan<const Id> children);
	// Link a list of nodes after prevChild in the list of children nodes
	void AddChildrenAfter(
	    TAccessRef<CChild, CParent> access, Id node, TSpan<Id> children, Id prevChild);
	void TransferChildren(TAccessRef<CChild, CParent> access, TSpan<Id> children, Id destination);
	// TODO: void TransferAllChildren(Tree& ast, Id origin, Id destination);
	void RemoveChildren(
	    TAccessRef<CParent, CChild> access, TSpan<Id> children, bool keepComponents);
	void RemoveAllChildren(
	    TAccessRef<CParent, CChild> access, TSpan<Id> parents, bool keepComponents = false);

	TArray<Id>* GetMutChildren(TAccessRef<CParent> access, Id node);
	const TArray<Id>* GetChildren(TAccessRef<const CParent> access, Id node);
	void GetChildren(
	    TAccessRef<const CParent> access, TSpan<const Id> nodes, TArray<Id>& outLinkedNodes);
	/**
	 * Finds all nodes connected recursively.
	 */
	void GetChildrenDeep(TAccessRef<const CParent> access, TSpan<const Id> roots,
	    TArray<Id>& outLinkedNodes, u32 depth = 0);
	Id GetParent(TAccessRef<const CChild> access, Id node);
	void GetParents(TAccessRef<const CChild> access, TSpan<Id> children, TArray<Id>& outParents);

	/**
	 * Find a parent id matching a delegate
	 */
	AST::Id FindParent(
	    TAccessRef<const CChild> access, AST::Id child, const TFunction<bool(AST::Id)>& callback);
	void FindParents(TAccessRef<const CChild> access, TSpan<Id> children, TArray<Id>& outParents,
	    const TFunction<bool(AST::Id)>& callback);

	// void Copy(Tree& ast, const TArray<Id>& nodes, TArray<Id>& outNewNodes);
	// void CopyDeep(Tree& ast, const TArray<Id>& rootNodes, TArray<Id>& outNewRootNodes);
	// void CopyAndTransferAllChildrenDeep(Tree& ast, Id root, Id otherRoot);

	void Remove(TAccessRef<CChild, CParent> access, TSpan<Id> nodes);
	void RemoveDeep(TAccessRef<CChild, CParent> access, TSpan<Id> nodes);

	/**
	 * Iterates children nodes making sure child->parent links are correct or fixed
	 * Only first depth links are affected
	 * Complexity: O(N)
	 * @parents: where to look for children to fix up
	 * @return true if an incorrect link was found and fixed
	 */
	bool FixParentLinks(TAccessRef<CChild, const CParent> access, TSpan<Id> parents);

	/**
	 * Iterates children nodes looking for invalid child->parent links
	 * Only first depth links are affected
	 * Complexity: O(1) <-> O(N) (First invalid link makes an early out)
	 * @parents: where to look for children
	 * @return true if an incorrect link was found
	 */
	bool ValidateParentLinks(TAccessRef<const CChild, const CParent> access, TSpan<Id> parents);
}    // namespace Rift::AST::Hierarchy
