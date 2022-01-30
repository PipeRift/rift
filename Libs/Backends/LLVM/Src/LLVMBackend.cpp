// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend.h"

//#include <llvm/IR/IRBuilder.h>
//#include <llvm/IR/LLVMContext.h>
//#include <llvm/IR/Module.h>


namespace Rift::Compiler
{
	void LLVMBackend::Build(Context& context)
	{
		// llvm::IRBuilder<> builder(TheContext);
		context.AddError("LLVM backend is not yet supported.");
	}
}    // namespace Rift::Compiler