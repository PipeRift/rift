// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Containers/ArrayView.h>


namespace Rift
{
	struct CParent;
	struct CChild;
}    // namespace Rift


namespace Rift::AST
{
	// Link a list of nodes at the end of the parent children list
	void Link(Tree& ast, Id node, TArrayView<const Id> children);
	// Link a list of nodes after prevChild in the list of children nodes
	void LinkAfter(Tree& ast, Id node, Id prevChild, TArrayView<Id> children);
	void TransferLinks(Tree& ast, TArrayView<Id> children, Id destination);
	void TransferAllLinks(Tree& ast, Id origin, Id destination);
	// void TransferAllLinks(Tree& ast, Id origin, Id destination, const TArray<Id>& children);
	void Unlink(Tree& ast, TArrayView<Id> children, bool keepComponents);
	void UnlinkAllChildren(Tree& ast, TArrayView<Id> parents, bool keepComponents = false);

	TArray<Id>* GetLinked(Tree& ast, Id node);
	const TArray<Id>* GetLinked(const Tree& ast, Id node);
	void GetLinked(const Tree& ast, TArrayView<const Id> nodes, TArray<Id>& outLinkedNodes);
	/**
	 * Finds all nodes connected recursively.
	 */
	void GetLinkedDeep(
	    const Tree& ast, TArrayView<const Id> roots, TArray<Id>& outLinkedNodes, u32 depth = 0);
	Id GetLinkedParent(Tree& ast, Id node);
	TArray<Id> GetLinkedParents(const Tree& ast, TArrayView<Id> nodes);

	// void Copy(Tree& ast, const TArray<Id>& nodes, TArray<Id>& outNewNodes);
	// void CopyDeep(Tree& ast, const TArray<Id>& rootNodes, TArray<Id>& outNewRootNodes);
	// void CopyAndTransferAllChildrenDeep(Tree& ast, Id root, Id otherRoot);

	void Remove(Tree& ast, TArrayView<Id> nodes);
	void RemoveDeep(Tree& ast, TArrayView<Id> nodes);

	/**
	 * @returns the CChild of a valid node. If the node doesnt have this component, nullptr is
	 * returned
	 */
	CChild* GetCChild(Tree& ast, Id node);
	const CChild* GetCChild(const Tree& ast, Id node);
	/**
	 * @returns the CParent of a valid node. If the node doesnt have this component, nullptr is
	 * returned
	 */
	CParent* GetCParent(Tree& ast, Id node);
	const CParent* GetCParent(const Tree& ast, Id node);

	/**
	 * Iterates children nodes making sure child->parent links are correct or fixed
	 * Only first depth links are affected
	 * Complexity: O(N)
	 * @parents: where to look for children to fix up
	 * @return true if an incorrect link was found and fixed
	 */
	bool FixParentLinks(Tree& ast, TArrayView<Id> parents);

	/**
	 * Iterates children nodes looking for invalid child->parent links
	 * Only first depth links are affected
	 * Complexity: O(1) <-> O(N) (First invalid link makes an early out)
	 * @parents: where to look for children
	 * @return true if an incorrect link was found
	 */
	bool ValidateParentLinks(const Tree& ast, TArrayView<Id> parents);
}    // namespace Rift::AST
