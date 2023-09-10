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
#include <Pipe/PipeECS.h>


namespace rift
{
	struct Compiler;
}

namespace rift::MIR
{
	// Defines a single ecs access dfor the entire IR generation
	using MIRAccess = p::TAccessRef<AST::CStmtOutput, AST::CStmtOutputs, AST::CExprInputs,
	    AST::CStmtIf, AST::CExprCallId, AST::CExprTypeId, AST::CExprOutputs, AST::CNamespace,
	    AST::CDeclType, AST::CDeclVariable, AST::CDeclStruct, AST::CDeclClass, AST::CDeclStatic,
	    AST::CParent, AST::CInvalid, AST::CChild, AST::CModule, AST::CLiteralBool,
	    AST::CLiteralIntegral, AST::CLiteralFloating, AST::CLiteralString, AST::CDeclFunction,
	    CDeclCStruct, CDeclCStatic, p::TWrite<CMIRType>, p::TWrite<CMIRFunctionSignature>,
	    p::TWrite<CMIRLiteral>>;

	void Generate(Compiler& compiler, MIR_context_t& ctx);


	struct CGenerator
	{
		Compiler& compiler;
		MIRAccess access;
		p::String* code = nullptr;

		void GenerateModule(AST::Id moduleId);

		void BindNativeTypes();
		void GenerateLiterals();

		void DeclareStructs(p::TView<AST::Id> ids);
		void DefineStructs(p::TView<AST::Id> ids);
		void DeclareFunctions(p::TView<AST::Id> ids, bool useFullName = true);
		void DefineFunctions(p::TView<AST::Id> ids);

		void AddStmtBlock(AST::Id firstStmtId);
		void AddStmtIf(AST::Id id);
		void AddExpr(const AST::ExprOutput& output);
		void AddCall(AST::Id id, const AST::CExprCallId& call);
		void CreateMain(AST::Id functionId);
		AST::Id FindMainFunction(p::TView<AST::Id> functionIds);
	};

}    // namespace rift::MIR
