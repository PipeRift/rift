// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Compiler/Backend.h>
#include <Plugin.h>
#include <RiftContext.h>


namespace rift
{
	class LLVMBackendPlugin : public Plugin
	{
		CLASS(LLVMBackendPlugin, Plugin)

	public:
		void Register(TPtr<RiftContext> context) override {}
	};


	namespace Compiler
	{
		class LLVMBackend : public Backend
		{
			CLASS(LLVMBackend, Backend)

		public:
			Name GetName() override
			{
				return "LLVM";
			}

			void Build(Context& context) override;
		};
	}    // namespace Compiler
}    // namespace rift
