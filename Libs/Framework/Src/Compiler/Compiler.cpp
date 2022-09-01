// Copyright 2015-2022 Piperift - All rights reserved

#include "Compiler/Compiler.h"

#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Utils/ModuleUtils.h"
#include "Compiler/Backend.h"
#include "Compiler/Systems/OptimizationSystem.h"
#include "Rift.h"

#include <Pipe/Files/Files.h>


namespace rift::compiler
{
	void Compiler::AddError(StringView str)
	{
		Log::Error(str);
		CompileError newError{};
		newError.text = str;
		errors.Add(newError);
	}


	void Build(AST::Tree& ast, const Config& config, TPtr<Backend> backend)
	{
		ZoneScoped;
		Compiler compiler{ast, config};

		if (!backend)
		{
			compiler.AddError("Invalid backend.");
			return;
		}

		{
			ZoneScopedN("Frontend");

			if (!AST::Modules::HasProject(ast))
			{
				Log::Error("No existing project to build.");
				return;
			}
			compiler.config.Init(ast);

			Log::Info("Loading files");
			AST::LoadSystem::Run(ast);

			OptimizationSystem::PruneDisconnectedExpressions(ast);
			AST::TypeSystem::PropagateVariableTypes(ast);
			AST::TypeSystem::PropagateExpressionTypes(ast);
		}

		Log::Info("Building project '{}'", AST::Modules::GetProjectName(compiler.ast));
		// Clean build folders
		Log::Info("Cleaning previous build");
		files::Delete(compiler.config.binariesPath, true, false);
		files::CreateFolder(compiler.config.binariesPath, true);

		backend->Build(compiler);
	}

	void Build(AST::Tree& ast, const Config& config, ClassType* backendType)
	{
		if (backendType)
		{
			TOwnPtr<Backend> backend = MakeOwned<Backend>(backendType);
			Build(ast, config, backend);
		}
	}
}    // namespace rift::compiler
