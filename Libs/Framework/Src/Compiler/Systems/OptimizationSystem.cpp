// Copyright 2015-2020 Piperift - All rights reserved

#include "Compiler/Systems/OptimizationSystem.h"

#include "AST/Utils/Hierarchy.h"


void Rift::OptimizationSystem::PruneDisconnected(AST::Tree& ast)
{
	TArray<AST::Id> nodesToRemove;

	// TODO: Gather type dangling nodes
	// auto childView = ast.Filter<CChild>();
	// ast.Each([&childView, &nodesToRemove](AST::Id id) {
	//	if (!childView.Has(id))
	//	{
	//		nodesToRemove.Add(id);
	//	}
	//});

	AST::Hierarchy::RemoveDeep(ast, nodesToRemove);
}
