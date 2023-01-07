// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <AST/Id.h>
#include <llvm/IR/Function.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct CIRFunction : public p::Struct
	{
		STRUCT(CIRFunction, p::Struct)

		llvm::Function* instance = nullptr;

		p::TArray<llvm::Value*> inputs;
		p::TArray<AST::Id> inputIds;


		CIRFunction() {}
	};
}    // namespace rift
