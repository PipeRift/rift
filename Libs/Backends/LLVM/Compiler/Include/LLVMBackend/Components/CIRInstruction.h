// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Instruction.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct CIRInstruction : public p::Struct
	{
		STRUCT(CIRInstruction, p::Struct)

		llvm::Instruction* instance = nullptr;


		CIRInstruction(llvm::Instruction* instance) : instance(instance) {}
	};
}    // namespace rift
