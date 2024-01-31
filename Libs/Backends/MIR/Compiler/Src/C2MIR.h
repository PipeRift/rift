// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Components.h"

#include <AST/Components/CModule.h>
#include <AST/Components/Expressions.h>
#include <AST/Components/Literals.h>
#include <AST/Components/Statements.h>
#include <AST/Components/Tags/CInvalid.h>
#include <AST/Id.h>
#include <Components/Declarations.h>
#include <mir.h>
#include <PipeECS.h>



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
