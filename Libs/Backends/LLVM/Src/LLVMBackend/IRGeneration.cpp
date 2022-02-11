// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend/IRGeneration.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>


namespace Rift::Compiler::LLVM
{
	void GenerateIR(Context& context)
	{
		llvm::LLVMContext llvmContext;
		llvm::IRBuilder<> builder(llvmContext);
		llvm::Module mod{"", llvmContext};
	}
}    // namespace Rift::Compiler::LLVM
