// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Types/Struct.h>


namespace Rift
{
	struct CIRStruct : public Struct
	{
		STRUCT(CIRStruct, Struct)

		llvm::StructType* instance = nullptr;


		CIRStruct(llvm::StructType* instance) : instance(instance) {}
	};
}    // namespace Rift
