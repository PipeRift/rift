// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Type.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CIRType : public Struct
	{
		STRUCT(CIRType, Struct)

		llvm::Type* instance = nullptr;


		CIRType(llvm::Type* instance) : instance(instance) {}
	};
}    // namespace Rift
