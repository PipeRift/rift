// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>


namespace rift::Compiler
{
	struct Context;
}

namespace rift::Compiler::LLVM
{
	struct ModuleIRGen
	{
		Context& compiler;
		llvm::Module& module;
		llvm::LLVMContext& llvm;
		llvm::IRBuilder<>& builder;
	};

	void GenerateIR(Context& context, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder);
}    // namespace rift::Compiler::LLVM
