// Copyright 2015-2020 Piperift - All rights reserved
#include "Compiler/Compiler.h"

#include "AST/Statics/SModules.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Utils/ModuleUtils.h"
#include "Compiler/CompilerContext.h"
#include "Compiler/Cpp/CppBackend.h"
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
			case EBackend::Cpp: break;
			case EBackend::LLVM: context.AddError("LLVM backend is not yet supported."); return;
			default: context.AddError("Unknown backend."); return;
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

		switch (backend)
		{
			case EBackend::Cpp: Cpp::Build(context); break;
		}
	}
}    // namespace Rift::Compiler
