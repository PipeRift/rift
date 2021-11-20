// Copyright 2015-2020 Piperift - All rights reserved

#include "Compiler/Systems/OptimizationSystem.h"

#include "AST/Linkage.h"


void Rift::OptimizationSystem::PruneDisconnected(AST::Tree& ast)
{
	TArray<AST::Id> nodesToRemove;

	// TODO: Gather type dangling nodes
	// auto childView = ast.Query<CChild>();
	// ast.Each([&childView, &nodesToRemove](AST::Id id) {
	//	if (!childView.Has(id))
	//	{
	//		nodesToRemove.Add(id);
	//	}
	//});

	AST::RemoveDeep(ast, nodesToRemove);
}
