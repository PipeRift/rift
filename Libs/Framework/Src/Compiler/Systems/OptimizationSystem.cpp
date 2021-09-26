// Copyright 2015-2020 Piperift - All rights reserved

#include "AST/Linkage.h"
#include "Compiler/Systems/OptimizationSystem.h"


void Rift::OptimizationSystem::PruneDisconnected(AST::Tree& ast)
{
	TArray<AST::Id> nodesToRemove;

	// TODO: Gather type dangling nodes
	// auto childView = ast.MakeView<CChild>();
	// ast.Each([&childView, &nodesToRemove](AST::Id id) {
	//	if (!childView.Has(id))
	//	{
	//		nodesToRemove.Add(id);
	//	}
	//});

	AST::RemoveDeep(ast, nodesToRemove);
}
