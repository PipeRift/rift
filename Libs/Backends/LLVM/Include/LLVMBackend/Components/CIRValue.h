// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Value.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CIRValue : public Struct
	{
		STRUCT(CIRValue, Struct)

		llvm::Value* instance = nullptr;


		CIRValue(llvm::Value* instance) : instance(instance) {}
	};
}    // namespace Rift
