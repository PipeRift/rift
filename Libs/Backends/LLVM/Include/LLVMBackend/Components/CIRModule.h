// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <llvm/IR/Module.h>
#include <Types/Struct.h>


namespace Rift
{
	struct CIRModule : public Struct
	{
		STRUCT(CIRModule, Struct)

		TOwnPtr<llvm::Module> instance;
	};
}    // namespace Rift
