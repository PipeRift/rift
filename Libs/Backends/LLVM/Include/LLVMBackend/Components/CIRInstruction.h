// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Instruction.h>
#include <Reflection/Struct.h>


namespace rift
{
	struct CIRInstruction : public pipe::Struct
	{
		STRUCT(CIRInstruction, pipe::Struct)

		llvm::Instruction* instance = nullptr;


		CIRInstruction(llvm::Instruction* instance) : instance(instance) {}
	};
}    // namespace rift
