// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Type.h>
#include <PRefl/Struct.h>


namespace rift
{
	struct CIRType : public p::Struct
	{
		STRUCT(CIRType, p::Struct)

		llvm::Type* instance = nullptr;


		CIRType(llvm::Type* instance) : instance(instance) {}
	};
}    // namespace rift
