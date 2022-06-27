// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend/Linker.h"

#include "LLVMBackend/Components/CIRModule.h"
#include "Pipe/Core/String.h"

#include <AST/Components/CIdentifier.h>
#include <AST/Components/CModule.h>
#include <AST/Utils/ModuleUtils.h>
#include <Pipe/Core/Subprocess.h>
#include <Pipe/ECS/Filtering.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Reflect/EnumType.h>


namespace rift::Compiler::LLVM
{
	void Link(Context& context)
	{
		for (AST::Id moduleId : ecs::ListAll<CModule, CIRModule>(context.ast))
		{
			p::Name moduleName = Modules::GetModuleName(context.ast, moduleId);
			const auto& module = context.ast.Get<const CModule>(moduleId);
			auto& irModule     = context.ast.Get<CIRModule>(moduleId);
			if (p::files::Exists(irModule.objectFile))
			{
				TArray<const char*> command{RIFT_LLVM_LINKER};
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
				    context.config.binariesPath / Strings::Format("{}.{}", moduleName, extension);
				String outParam = Strings::Format("/out:{}", p::ToString(filePath));
				command.Add(outParam.data());

				Log::Info("Linking '{}' from '{}'", p::ToString(filePath), irModule.objectFile);
				p::RunProcess(command,
				    SubprocessOptions::TerminateIfDestroyed | SubprocessOptions::CombinedOutErr);
			}
		}
	}
}    // namespace rift::Compiler::LLVM
