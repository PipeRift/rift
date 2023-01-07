// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Type.h>
#include <Pipe/Reflect/Struct.h>


namespace rift
{
	struct CIRType : public p::Struct
	{
		STRUCT(CIRType, p::Struct)

		llvm::Type* instance = nullptr;


		CIRType(llvm::Type* instance) : instance(instance) {}
	};
}    // namespace rift
