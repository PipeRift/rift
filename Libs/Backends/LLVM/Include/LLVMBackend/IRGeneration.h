// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>


namespace Rift::Compiler
{
	struct Context;
}

namespace Rift::Compiler::LLVM
{
	void GenerateIR(Context& context, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder);
}
