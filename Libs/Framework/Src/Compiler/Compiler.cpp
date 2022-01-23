// Copyright 2015-2022 Piperift - All rights reserved
#include "Compiler/Compiler.h"

#include "AST/Statics/SModules.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Utils/ModuleUtils.h"
#include "Compiler/Context.h"
#include "Compiler/Systems/OptimizationSystem.h"
#include "RiftContext.h"


namespace Rift::Compiler
{
	void Build(AST::Tree& ast, const Config& config, TPtr<Backend> backend)
	{
		Context context{ast, config};

		if (!backend)
		{
			context.AddError("Invalid backend.");
			return;
		}

		LoadSystem::Init(ast);
		TypeSystem::Init(ast);

		if (!Modules::HasProject(ast))
		{
			Log::Error("No existing project to build.");
			return;
		}
		context.config.Init(ast);

		Log::Info("Loading files");
		LoadSystem::Run(ast);

		OptimizationSystem::PruneDisconnectedExpressions(ast);
		TypeSystem::RunChecks(ast);

		backend->Build(context);
	}
}    // namespace Rift::Compiler
