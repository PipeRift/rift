// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend/IRGeneration.h"

#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/Components/CIRStruct.h"
#include "LLVMBackend/LLVMHelpers.h"

#include <AST/Components/CClassDecl.h>
#include <AST/Components/CLiteralFloat.h>
#include <AST/Components/CStructDecl.h>
#include <AST/Components/CType.h>
#include <AST/Filtering.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
#include <llvm/IR/Module.h>


namespace Rift::Compiler::LLVM
{
	llvm::Value* GetLiteralFloat(
	    llvm::LLVMContext& llvm, AST::TAccessRef<const CLiteralFloat> access, AST::Id id)
	{
		return llvm::ConstantFP::get(
		    llvm, llvm::APFloat(access.Get<const CLiteralFloat>(id).value));
	}

	void DeclareStruct(
	    llvm::LLVMContext& llvm, AST::TAccessRef<const CType, CIRStruct> access, AST::Id structId)
	{
		const Name name = access.Get<const CType>(structId).name;
		access.Add(structId, CIRStruct{llvm::StructType::create(llvm, ToLLVM(name))});
	}

	void DefineStruct(
	    llvm::LLVMContext& llvm, AST::TAccessRef<const CIRStruct> access, AST::Id structId)
	{
		llvm::StructType* irStruct = access.Get<const CIRStruct>(structId).instance;

		TArray<llvm::Type*> memberTypes;
		irStruct->setBody(ToLLVM(memberTypes));
	}

	void GenerateIRModule(
	    Context& context, AST::Id moduleId, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		const auto& config = context.config;
		auto& ast          = context.ast;

		const Name name = Modules::GetModuleName(context.ast, moduleId);

		CIRModule& mod         = context.ast.Add<CIRModule>(moduleId);
		mod.instance           = Move(MakeOwned<llvm::Module>(ToLLVM(name), llvm));
		llvm::Module& irModule = *mod.instance.Get();

		TArray<AST::Id> classes, structs;
		AST::Hierarchy::GetChildren(ast, moduleId, classes);
		AST::RemoveIfNot<CType>(ast, classes);
		structs = classes;

		AST::RemoveIfNot<CClassDecl>(ast, classes);
		AST::RemoveIfNot<CStructDecl>(ast, structs);


		// Declare types
		AST::TAccess<const CType, CIRStruct> declareAccess{ast};
		for (AST::Id structId : structs)
		{
			DeclareStruct(llvm, declareAccess, structId);
		}
		for (AST::Id classId : classes)
		{
			DeclareStruct(llvm, declareAccess, classId);
		}

		// Declare functions

		// Define types
		AST::TAccess<const CIRStruct> defineAccess{ast};
		for (AST::Id structId : structs)
		{
			DefineStruct(llvm, defineAccess, structId);
		}
		for (AST::Id classId : classes)
		{
			DefineStruct(llvm, defineAccess, classId);
		}

		// Define functions
	}

	void GenerateIR(Context& context, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		for (AST::Id moduleId : AST::ListAll<CModule>(context.ast))
		{
			GenerateIRModule(context, moduleId, llvm, builder);
		}
	}
}    // namespace Rift::Compiler::LLVM
