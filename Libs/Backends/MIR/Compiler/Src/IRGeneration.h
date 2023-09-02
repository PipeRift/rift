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
#include <mir.h>
#include <Pipe/PipeECS.h>


namespace rift
{
	struct CIRFunction;
}

namespace rift
{
	struct Compiler;
}

namespace rift::MIR
{
	// Defines a single ecs access dfor the entire IR generation
	using MIRAccess = p::TAccessRef<AST::CStmtOutput, AST::CStmtOutputs, AST::CExprInputs,
	    AST::CStmtIf, AST::CExprCallId, AST::CExprTypeId, AST::CExprOutputs, AST::CNamespace,
	    AST::CDeclType, AST::CDeclVariable, AST::CParent, AST::CInvalid, AST::CChild, AST::CModule,
	    p::TWrite<CMIRType>, AST::CLiteralBool, AST::CLiteralIntegral, AST::CLiteralFloating,
	    AST::CLiteralString>;

	struct ModuleIRGen
	{
		Compiler& compiler;
		MIR_context_t& ctx;
		MIR_module_t& module;
	};

	void GenerateIR(Compiler& compiler, MIR_context_t& ctx);

	void GenerateIRModule(
	    Compiler& compiler, MIRAccess access, AST::Id moduleId, MIR_context_t& ctx);

	void BindNativeTypes(MIR_context_t& ctx, MIRAccess access);
}    // namespace rift::MIR
