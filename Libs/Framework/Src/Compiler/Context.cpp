// Copyright 2015-2022 Piperift - All rights reserved

#include "Compiler/Context.h"


namespace rift::Compiler
{
	void Context::AddError(StringView str)
	{
		Log::Error(str);
		CompileError newError{};
		newError.text = str;
		errors.Add(newError);
	}
}    // namespace rift::Compiler
