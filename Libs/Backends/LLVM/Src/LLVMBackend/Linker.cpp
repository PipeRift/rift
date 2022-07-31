// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend/Linker.h"

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


namespace rift::compiler::LLVM
{
	void Link(Compiler& compiler)
	{
		for (AST::Id moduleId : ecs::ListAll<CModule, CIRModule>(compiler.ast))
		{
			p::Name moduleName = Modules::GetModuleName(compiler.ast, moduleId);
			const auto& module = compiler.ast.Get<const CModule>(moduleId);
			auto& irModule     = compiler.ast.Get<CIRModule>(moduleId);
			if (p::files::Exists(irModule.objectFile))
			{
				TArray<const char*> command;

				String linkerPath{PlatformProcess::GetExecutablePath()};
				linkerPath.append("/");
				linkerPath.append(RIFT_LLVM_LINKER_PATH);
				command.Add(linkerPath.c_str());

				const char* extension = nullptr;
				switch (module.target)
				{
					case ModuleTarget::Executable:
						command.Add("/entry:main");
						extension = "exe";
						break;
					case ModuleTarget::Shared:
						command.Add("/dll");
						extension = "dll";
						break;
					case ModuleTarget::Static:
						command.Add("/lib");
						extension = "lib";
						break;
				}
				command.Add(irModule.objectFile.data());

				p::Path filePath =
				    compiler.config.binariesPath / Strings::Format("{}.{}", moduleName, extension);
				String outParam = Strings::Format("/out:{}", p::ToString(filePath));
				command.Add(outParam.data());

				Log::Info("Linking '{}' from '{}'", p::ToString(filePath), irModule.objectFile);
				p::RunProcess(command,
				    SubprocessOptions::TerminateIfDestroyed | SubprocessOptions::CombinedOutErr);
			}
		}
	}
}    // namespace rift::compiler::LLVM
