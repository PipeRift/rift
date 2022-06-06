// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Value.h>
#include <PRefl/Struct.h>


namespace rift
{
	struct CIRValue : public p::Struct
	{
		STRUCT(CIRValue, p::Struct)

		llvm::Value* instance = nullptr;


		CIRValue(llvm::Value* instance) : instance(instance) {}
	};
}    // namespace rift
