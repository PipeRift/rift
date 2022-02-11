// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend.h"

#include "LLVMBackend/IRGeneration.h"


namespace Rift::Compiler
{
	void LLVMBackend::Build(Context& context)
	{
		ZoneScopedC(0x459bd1);

		Log::Info("Generating LLVM IR");
		LLVM::GenerateIR(context);
	}
}    // namespace Rift::Compiler