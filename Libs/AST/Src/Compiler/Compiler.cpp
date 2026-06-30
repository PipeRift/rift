// Copyright 2015-2026 Piperift. All Rights Reserved.

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
	void Compiler::Error(p::StringView str)
	{
		p::Error(str);
		CompileError newError{};
		newError.text = str;
		errors.Add(newError);
	}


	void Build(ast::Tree& ast, const CompilerConfig& config, p::TPtr<Backend> backend)
	{
		Compiler compiler{ast, config};

		if (!backend)
		{
			compiler.Error("Invalid backend.");
			return;
		}

		if (!ast::HasProject(ast))
		{
			p::Error("No existing project to build.");
			return;
		}

		compiler.config.Init(ast);

		if (auto* nativeBindings = GetModule<NativeBindingModule>().Get())
		{
			p::Info("Interpret native modules");
			nativeBindings->SyncIncludes(ast);
		}

		p::Info("Loading files");
		ast::LoadSystem::Run(ast);
		ast::TypeSystem::SyncTypesByPath(ast);

		OptimizationSystem::PruneDisconnectedExpressions(ast);
		ast::TypeSystem::PropagateVariableTypes(ast);
		ast::TypeSystem::PropagateExpressionTypes(ast);


		p::Info("Building project '{}'", ast::GetProjectName(compiler.ast));
		// Clean build folders
		p::Info("Cleaning previous build");
		Delete(compiler.config.binariesPath, true, false);
		CreateFolder(compiler.config.binariesPath, true);

		backend->Build(compiler);
	}

	void Build(ast::Tree& ast, const CompilerConfig& config, p::TypeId backendType)
	{
		if (backendType.IsValid())
		{
			p::TOwnPtr<Backend> backend = p::MakeOwned<Backend>(backendType);
			Build(ast, config, backend);
		}
	}
}    // namespace rift
