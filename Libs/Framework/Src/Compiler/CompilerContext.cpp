// Copyright 2015-2020 Piperift - All rights reserved

#include "Compiler/CompilerContext.h"


namespace Rift::Compiler
{
	void Context::AddError(StringView str)
	{
		Log::Error(str);
		CompileError newError{};
		newError.text = str;
		errors.Add(newError);
	}
}    // namespace Rift::Compiler
