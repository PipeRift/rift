// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Id.h>
#include <llvm/IR/Function.h>
#include <Reflection/Struct.h>


namespace rift
{
	struct CIRFunction : public pipe::Struct
	{
		STRUCT(CIRFunction, pipe::Struct)

		llvm::Function* instance = nullptr;

		pipe::TArray<llvm::Value*> inputs;
		pipe::TArray<AST::Id> inputIds;


		CIRFunction() {}
	};
}    // namespace rift
