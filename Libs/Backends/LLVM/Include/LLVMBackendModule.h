// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Compiler/Backend.h>
#include <Module.h>
#include <Rift.h>


namespace rift
{
	class LLVMBackendModule : public Module
	{
		CLASS(LLVMBackendModule, Module)

	public:
		LLVMBackendModule();
	};


	namespace compiler
	{
		class LLVMBackend : public Backend
		{
			CLASS(LLVMBackend, Backend)

		public:
			Name GetName() override
			{
				return "LLVM";
			}

			void Build(Compiler& compiler) override;
		};
	}    // namespace compiler
}    // namespace rift
