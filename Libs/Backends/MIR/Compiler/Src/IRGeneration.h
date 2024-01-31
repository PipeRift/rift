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
#include <PipeECS.h>



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

	void GenerateC(Compiler& compiler);


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
