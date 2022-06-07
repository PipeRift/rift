// Copyright 2015-2022 Piperift - All rights reserved

#include "Compiler/Compiler.h"

#include "AST/Statics/SModules.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Utils/ModuleUtils.h"
#include "Compiler/Context.h"
#include "Compiler/Systems/OptimizationSystem.h"
#include "RiftContext.h"

#include <Pipe/Files/Files.h>


namespace rift::Compiler
{
	void Build(AST::Tree& ast, const Config& config, TPtr<Backend> backend)
	{
		ZoneScoped;

		Context context{ast, config};

		if (!backend)
		{
			context.AddError("Invalid backend.");
			return;
		}

		{
			ZoneScopedN("Frontend");

			if (!Modules::HasProject(ast))
			{
				Log::Error("No existing project to build.");
				return;
			}
			context.config.Init(ast);

			Log::Info("Loading files");
			LoadSystem::Run(ast);

			OptimizationSystem::PruneDisconnectedExpressions(ast);
			TypeSystem::PropagateVariableTypes(ast);
			TypeSystem::PropagateExpressionTypes(ast);
		}

		Log::Info("Building project '{}'", Modules::GetProjectName(context.ast));
		// Clean build folders
		Log::Info("Cleaning previous build");
		files::Delete(context.config.binariesPath, true, false);
		files::CreateFolder(context.config.binariesPath, true);

		backend->Build(context);
	}
}    // namespace rift::Compiler
