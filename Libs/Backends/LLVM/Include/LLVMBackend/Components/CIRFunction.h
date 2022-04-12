// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Function.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CIRFunction : public Struct
	{
		STRUCT(CIRFunction, Struct)

		llvm::Function* instance = nullptr;


		CIRFunction(llvm::Function* instance) : instance(instance) {}
	};
}    // namespace Rift
