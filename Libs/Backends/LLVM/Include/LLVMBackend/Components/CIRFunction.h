// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <AST/Id.h>
#include <llvm/IR/Function.h>
#include <Reflection/Struct.h>


namespace Rift
{
	struct CIRFunction : public Pipe::Struct
	{
		STRUCT(CIRFunction, Pipe::Struct)

		llvm::Function* instance = nullptr;

		Pipe::TArray<llvm::Value*> inputs;
		Pipe::TArray<AST::Id> inputIds;


		CIRFunction() {}
	};
}    // namespace Rift
