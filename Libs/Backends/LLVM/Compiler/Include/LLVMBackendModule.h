// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Compiler/Backend.h>
#include <Module.h>


namespace rift
{
	class LLVMBackendModule : public Module
	{
		CLASS(LLVMBackendModule, Module)

	public:
		LLVMBackendModule();
	};


	class LLVMBackend : public Backend
	{
		CLASS(LLVMBackend, Backend)

	public:
		Tag GetName() override
		{
			return "LLVM";
		}

		void Build(Compiler& compiler) override;
	};
}    // namespace rift
