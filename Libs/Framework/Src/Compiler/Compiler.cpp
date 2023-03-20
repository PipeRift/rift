// Copyright 2015-2023 Piperift - All rights reserved

#include "Compiler/Compiler.h"

#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Utils/ModuleUtils.h"
#include "Compiler/Backend.h"
#include "Compiler/Systems/OptimizationSystem.h"
#include "Rift.h"

#include <NativeBindingModule.h>
#include <Pipe/Files/Files.h>


namespace rift
{
	void Compiler::AddError(StringView str)
	{
		Log::Error(str);
		CompileError newError{};
		newError.text = str;
		errors.Add(newError);
	}


	void Build(AST::Tree& ast, const CompilerConfig& config, TPtr<Backend> backend)
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

			if (!AST::HasProject(ast))
			{
				Log::Error("No existing project to build.");
				return;
			}
			compiler.config.Init(ast);

			if (auto* nativeBindings = GetModule<NativeBindingModule>().Get())
			{
				Log::Info("Interpret native modules");
				nativeBindings->SyncIncludes(ast);
			}

			Log::Info("Loading files");
			AST::LoadSystem::Run(ast);

			OptimizationSystem::PruneDisconnectedExpressions(ast);
			AST::TypeSystem::PropagateVariableTypes(ast);
			AST::TypeSystem::PropagateExpressionTypes(ast);
		}

		Log::Info("Building project '{}'", AST::GetProjectName(compiler.ast));
		// Clean build folders
		Log::Info("Cleaning previous build");
		files::Delete(compiler.config.binariesPath, true, false);
		files::CreateFolder(compiler.config.binariesPath, true);

		backend->Build(compiler);
	}

	void Build(AST::Tree& ast, const CompilerConfig& config, ClassType* backendType)
	{
		if (backendType)
		{
			TOwnPtr<Backend> backend = MakeOwned<Backend>(backendType);
			Build(ast, config, backend);
		}
	}
}    // namespace rift
