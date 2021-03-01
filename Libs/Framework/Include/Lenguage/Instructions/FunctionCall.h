// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Assets/TypeAsset.h"

#include <Object/Struct.h>


namespace Rift
{
	/**
	 * Instruction that calls a C api function
	 */
	struct NativeFunctionCall : public Instruction
	{
		STRUCT(NativeFunctionCall, Instruction)
	};

	/**
	 * Instruction that calls a Rift function
	 */
	struct FunctionCall : public Instruction
	{
		STRUCT(FunctionCall, Instruction)
	};
}    // namespace Rift
