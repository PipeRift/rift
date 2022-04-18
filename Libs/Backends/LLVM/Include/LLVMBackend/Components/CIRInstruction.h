// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Instruction.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CIRInstruction : public Struct
	{
		STRUCT(CIRInstruction, Struct)

		llvm::Instruction* instance = nullptr;


		CIRInstruction(llvm::Instruction* instance) : instance(instance) {}
	};
}    // namespace Rift
