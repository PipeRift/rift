// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend.h"

#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/IRGeneration.h"

#include <AST/Filtering.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Module.h>


namespace Rift::Compiler
{
	void LLVMBackend::Build(Context& context)
	{
		ZoneScopedC(0x459bd1);

		Log::Info("Generating LLVM IR");
		LLVM::GenerateIR(context);

		Log::Info("Creating LLVM Bitcode");
		// const Path bitCodePath = context.config.intermediatesPath / "LLVM";
		for (AST::Id moduleId : AST::ListAll<CIRModule>(context.ast))
		{
			const auto& irModule = context.ast.Get<const CIRModule>(moduleId).instance;
			llvm::WriteBitcodeToFile(*irModule.Get(), llvm::outs());
		}
	}
}    // namespace Rift::Compiler