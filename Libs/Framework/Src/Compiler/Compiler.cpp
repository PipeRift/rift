// Copyright 2015-2020 Piperift - All rights reserved
#include "AST/Systems/AssetSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "Compiler/Compiler.h"
#include "Compiler/CompilerContext.h"
#include "Compiler/Cpp/CppBackend.h"
#include "Compiler/Systems/CompileTimeSystem.h"
#include "Compiler/Systems/OptimizationSystem.h"
#include "RiftContext.h"


namespace Rift::Compiler
{
	void Build(TPtr<Module> project, const Config& config, EBackend backend)
	{
		if (!project)
		{
			return;
		}

		// TODO: Provide build independant ast

		AssetSystem::Init(RiftContext::AST());
		TypeSystem::Init(RiftContext::AST());
		OptimizationSystem::Init(RiftContext::AST());
		CompileTimeSystem::Init(RiftContext::AST());

		AssetSystem::Run(RiftContext::AST());
		TypeSystem::Run(RiftContext::AST());
		OptimizationSystem::Run(RiftContext::AST());
		CompileTimeSystem::Run(RiftContext::AST());

		switch (backend)
		{
			case EBackend::Cpp:
				Cpp::Build(project, config);
				break;
			case EBackend::LLVM:
				Log::Error("LLVM backend is not yet supported.");
				break;
			default:
				Log::Error("Unknown backend.");
		}
	}
}    // namespace Rift::Compiler
