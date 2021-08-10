// Copyright 2015-2020 Piperift - All rights reserved
#include "Compiler/Compiler.h"

#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/ModuleSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Uniques/CModulesUnique.h"
#include "Compiler/CompilerContext.h"
#include "Compiler/Cpp/CppBackend.h"
#include "Compiler/Systems/CompileTimeSystem.h"
#include "Compiler/Systems/OptimizationSystem.h"
#include "RiftContext.h"


namespace Rift::Compiler
{
	void Build(AST::Tree& ast, const Config& config, EBackend backend)
	{
		Context context{ast, config};

		// Early backend check
		switch (backend)
		{
			case EBackend::Cpp:
				break;
			case EBackend::LLVM:
				context.AddError("LLVM backend is not yet supported.");
				return;
			default:
				context.AddError("Unknown backend.");
				return;
		}

		TypeSystem::Init(ast);
		OptimizationSystem::Init(ast);
		CompileTimeSystem::Init(ast);
		LoadSystem::Init(ast);

		auto* modules = ast.TryGetUnique<CModulesUnique>();
		if (!modules || !modules->HasMainModule())
		{
			Log::Error("No existing module selected to build.");
			return;
		}
		context.config.Init(ast);

		ModuleSystem::ScanSubmodules(ast);
		ModuleSystem::ScanModuleTypes(ast);
		Log::Info("Loading files");
		LoadSystem::Run(ast);

		TypeSystem::RunChecks(ast);
		OptimizationSystem::Run(ast);
		CompileTimeSystem::Run(ast);

		switch (backend)
		{
			case EBackend::Cpp:
				Cpp::Build(context);
				break;
		}
	}
}    // namespace Rift::Compiler
