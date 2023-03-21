// Copyright 2015-2023 Piperift - All rights reserved

#include "LLVMBackend/Linker.h"

#include "Components/CNativeBinding.h"
#include "LLVMBackend/Components/CIRModule.h"
#include "Pipe/Core/PlatformProcess.h"
#include "Pipe/Core/String.h"

#include <AST/Components/CModule.h>
#include <AST/Components/CNamespace.h>
#include <AST/Utils/ModuleUtils.h>
#include <Pipe/Core/Subprocess.h>
#include <Pipe/ECS/Filtering.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/EnumType.h>


namespace rift::LLVM
{
	void Link(Compiler& compiler)
	{
		String linkerPath{
		    p::JoinPaths(PlatformProcess::GetExecutablePath(), RIFT_LLVM_LINKER_PATH)};

		for (AST::Id moduleId : ecs::ListAll<AST::CModule, CIRModule>(compiler.ast))
		{
			p::Tag moduleName  = AST::GetModuleName(compiler.ast, moduleId);
			const auto& module = compiler.ast.Get<const AST::CModule>(moduleId);
			auto& irModule     = compiler.ast.Get<CIRModule>(moduleId);
			if (p::files::Exists(irModule.objectFile))
			{
				TArray<const char*> command;
				command.Add(linkerPath.c_str());

				const char* extension = nullptr;
				switch (module.target)
				{
					case AST::RiftModuleTarget::Executable:
						command.Add("/entry:Main");
						command.Add("/SUBSYSTEM:CONSOLE");
						extension = "exe";
						break;
					case AST::RiftModuleTarget::Shared:
						command.Add("/dll");
						extension = "dll";
						break;
					case AST::RiftModuleTarget::Static:
						command.Add("/lib");
						extension = "lib";
						break;
				}

				p::TArray<p::String> binaryPaths;
				if (auto* cBinding = compiler.ast.TryGet<CNativeBinding>(moduleId))
				{
					p::StringView modulePath = AST::GetModulePath(compiler.ast, moduleId);
					p::String binaryPath;
					for (const auto& nativeBinary : cBinding->binaries)
					{
						binaryPaths.Add(p::JoinPaths(modulePath, nativeBinary));
						command.Add(binaryPaths.Last().c_str());
					}
				}
				command.Add(irModule.objectFile.data());

				p::Path filePath =
				    compiler.config.binariesPath / Strings::Format("{}.{}", moduleName, extension);
				String outParam = Strings::Format("/out:{}", p::ToString(filePath));
				command.Add(outParam.data());

				Log::Info("Linking '{}' from '{}'", p::ToString(filePath), irModule.objectFile);
				auto process   = p::RunProcess(command,
				      SubprocessOptions::TerminateIfDestroyed | SubprocessOptions::CombinedOutErr);
				i32 returnCode = 0;
				p::WaitProcess(process.TryGet(), &returnCode);
				if (returnCode != 0)
				{
					compiler.AddError("Linking failed");
				}
			}
		}
	}
}    // namespace rift::LLVM
