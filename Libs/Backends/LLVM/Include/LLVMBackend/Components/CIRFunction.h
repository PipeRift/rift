// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Types.h>
#include <llvm/IR/Function.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CIRFunction : public Struct
	{
		STRUCT(CIRFunction, Struct)

		llvm::Function* instance = nullptr;

		TArray<llvm::Value*> inputs;
		TArray<AST::Id> inputIds;


		CIRFunction() {}
	};
}    // namespace Rift
