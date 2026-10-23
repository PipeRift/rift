// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "Components.h"


struct c2mir_options;

namespace rift
{
	struct Compiler;
}

namespace rift::MIR
{
	void CToMIR(Compiler& compiler, MIR_context* ctx);
	void CToMIRModule(Compiler& compiler, MIR_context* ctx, c2mir_options& options, p::Tag name,
	    const CMIRModule& module);
}    // namespace rift::MIR
