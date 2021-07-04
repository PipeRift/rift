// Copyright 2015-2020 Piperift - All rights reserved
#include "AST/Systems/ModuleSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Uniques/CModulesUnique.h"
#include "Compiler/Compiler.h"
#include "Compiler/CompilerContext.h"
#include "Compiler/Cpp/CppBackend.h"
#include "Compiler/Systems/CompileTimeSystem.h"
#include "Compiler/Systems/OptimizationSystem.h"
#include "RiftContext.h"


namespace Rift::Compiler
{
	void Build(AST::Tree& ast, const Config& config, EBackend backend)
	{
		Context context{ast};

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

		ModuleSystem::Init(ast);
		TypeSystem::Init(ast);
		OptimizationSystem::Init(ast);
		CompileTimeSystem::Init(ast);

		ModuleSystem::Run(ast);
		auto& modules = ast.GetUnique<CModulesUnique>();
		if (modules.HasMainModule())
		{
			Log::Error("No existing module selected to build.");
			return;
		}

		TypeSystem::RunChecks(ast);
		OptimizationSystem::Run(ast);
		CompileTimeSystem::Run(ast);

		switch (backend)
		{
			case EBackend::Cpp:
				Cpp::Build(context, config);
				break;
		}
	}
}    // namespace Rift::Compiler
