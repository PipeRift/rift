// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>


namespace rift::compiler
{
	struct Compiler;
}

namespace rift::compiler::LLVM
{
	struct ModuleIRGen
	{
		Compiler& compiler;
		llvm::Module& module;
		llvm::LLVMContext& llvm;
		llvm::IRBuilder<>& builder;
	};

	void GenerateIR(Compiler& compiler, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder);
}    // namespace rift::compiler::LLVM
