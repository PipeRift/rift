// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Compiler/Backend.h>
#include <Plugin.h>
#include <Rift.h>


namespace rift
{
	class LLVMBackendPlugin : public Plugin
	{
		CLASS(LLVMBackendPlugin, Plugin)

	public:
		void Register(TPtr<Rift> rift) override {}
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
