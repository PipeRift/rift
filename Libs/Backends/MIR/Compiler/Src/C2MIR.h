// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Components.h"

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclStatic.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CDeclType.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprCall.h>
#include <AST/Components/CExprInputs.h>
#include <AST/Components/CExprOutputs.h>
#include <AST/Components/CExprType.h>
#include <AST/Components/CLiteralBool.h>
#include <AST/Components/CLiteralFloating.h>
#include <AST/Components/CLiteralIntegral.h>
#include <AST/Components/CLiteralString.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CStmtIf.h>
#include <AST/Components/CStmtOutputs.h>
#include <AST/Components/CStmtReturn.h>
#include <AST/Components/Tags/CInvalid.h>
#include <AST/Id.h>
#include <Components/CDeclCStatic.h>
#include <Components/CDeclCStruct.h>
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
