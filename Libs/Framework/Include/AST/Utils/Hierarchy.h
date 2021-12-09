// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Containers/Span.h>


namespace Rift
{
	struct CParent;
	struct CChild;
}    // namespace Rift


namespace Rift::AST::Hierarchy
{
	// Link a list of nodes at the end of the parent children list
	void AddChildren(Tree& ast, Id node, TSpan<const Id> children);
	// Link a list of nodes after prevChild in the list of children nodes
	void AddChildrenAfter(Tree& ast, Id node, TSpan<Id> children, Id prevChild);
	void TransferChildren(Tree& ast, TSpan<Id> children, Id destination);
	void TransferAllChildren(Tree& ast, Id origin, Id destination);
	void RemoveChildren(Tree& ast, TSpan<Id> children, bool keepComponents);
	void RemoveAllChildren(Tree& ast, TSpan<Id> parents, bool keepComponents = false);

	TArray<Id>* GetChildren(Tree& ast, Id node);
	const TArray<Id>* GetChildren(const Tree& ast, Id node);
	void GetChildren(const Tree& ast, TSpan<const Id> nodes, TArray<Id>& outLinkedNodes);
	/**
	 * Finds all nodes connected recursively.
	 */
	void GetChildrenDeep(
	    const Tree& ast, TSpan<const Id> roots, TArray<Id>& outLinkedNodes, u32 depth = 0);
	Id GetParent(Tree& ast, Id node);
	TArray<Id> GetParents(const Tree& ast, TSpan<Id> nodes);

	// void Copy(Tree& ast, const TArray<Id>& nodes, TArray<Id>& outNewNodes);
	// void CopyDeep(Tree& ast, const TArray<Id>& rootNodes, TArray<Id>& outNewRootNodes);
	// void CopyAndTransferAllChildrenDeep(Tree& ast, Id root, Id otherRoot);

	void Remove(Tree& ast, TSpan<Id> nodes);
	void RemoveDeep(Tree& ast, TSpan<Id> nodes);

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
	bool FixParentLinks(Tree& ast, TSpan<Id> parents);

	/**
	 * Iterates children nodes looking for invalid child->parent links
	 * Only first depth links are affected
	 * Complexity: O(1) <-> O(N) (First invalid link makes an early out)
	 * @parents: where to look for children
	 * @return true if an incorrect link was found
	 */
	bool ValidateParentLinks(const Tree& ast, TSpan<Id> parents);
}    // namespace Rift::AST::Hierarchy
