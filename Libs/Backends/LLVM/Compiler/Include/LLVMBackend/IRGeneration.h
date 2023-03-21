// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "LLVMBackend/Components/CIRFunction.h"
#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/Components/CIRType.h"
#include "LLVMBackend/Components/CIRValue.h"

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
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <Pipe/ECS/Filtering.h>


namespace rift
{
	struct CIRFunction;
}

namespace rift
{
	struct Compiler;
}

namespace rift::LLVM
{
	// Defines a single ecs access dfor the entire IR generation
	using IRAccess = p::TAccessRef<AST::CStmtOutput, AST::CStmtOutputs, AST::CExprInputs,
	    AST::CStmtIf, AST::CExprCallId, AST::CExprTypeId, AST::CExprOutputs, AST::CNamespace,
	    AST::CDeclType, AST::CDeclVariable, AST::CParent, AST::CInvalid, AST::CChild, AST::CModule,
	    p::TWrite<CIRValue>, p::TWrite<CIRType>, p::TWrite<CIRFunction>, AST::CLiteralBool,
	    AST::CLiteralIntegral, AST::CLiteralFloating, AST::CLiteralString>;

	struct ModuleIRGen
	{
		Compiler& compiler;
		llvm::Module& module;
		llvm::LLVMContext& llvm;
		llvm::IRBuilder<>& builder;
	};

	void GenerateIR(Compiler& compiler, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder);

	void GenerateIRModule(Compiler& compiler, IRAccess access, AST::Id moduleId,
	    llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder);

	void BindNativeTypes(llvm::LLVMContext& llvm, IRAccess access);
	void GenerateLiterals(llvm::LLVMContext& llvm, IRAccess access);

	void DeclareStructs(ModuleIRGen& gen, IRAccess access, p::TSpan<AST::Id> ids);
	void DefineStructs(ModuleIRGen& gen, IRAccess access, p::TSpan<AST::Id> ids);
	void DeclareFunctions(ModuleIRGen& gen, IRAccess access, p::TSpan<AST::Id> ids);
	void DefineFunctions(ModuleIRGen& gen, IRAccess access, p::TSpan<AST::Id> ids);

	void AddStmtBlock(ModuleIRGen& gen, IRAccess access, AST::Id firstStmtId,
	    llvm::BasicBlock* block, const CIRFunction& function);
	llvm::Value* AddExpr(ModuleIRGen& gen, IRAccess access, const AST::ExprOutput& output);
	llvm::BasicBlock* AddIf(
	    ModuleIRGen& gen, IRAccess access, AST::Id id, const CIRFunction& function);
	void AddCall(ModuleIRGen& gen, AST::Id id, const AST::CExprCallId& call, IRAccess access);


	AST::Id FindMainFunction(IRAccess access, p::TSpan<AST::Id> functionIds);

	void CreateMain(ModuleIRGen& gen, IRAccess access, AST::Id functionId);
}    // namespace rift::LLVM
