// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Instruction.h>
#include <Reflection/Struct.h>


namespace Rift
{
	struct CIRInstruction : public Pipe::Struct
	{
		STRUCT(CIRInstruction, Pipe::Struct)

		llvm::Instruction* instance = nullptr;


		CIRInstruction(llvm::Instruction* instance) : instance(instance) {}
	};
}    // namespace Rift
