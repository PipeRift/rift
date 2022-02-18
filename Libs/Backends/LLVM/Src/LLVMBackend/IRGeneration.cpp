// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend/IRGeneration.h"

#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/Components/CIRStruct.h"
#include "LLVMBackend/LLVMHelpers.h"

#include <AST/Components/CClassDecl.h>
#include <AST/Components/CLiteralFloat.h>
#include <AST/Components/CLiteralI32.h>
#include <AST/Components/CStructDecl.h>
#include <AST/Components/CType.h>
#include <AST/Filtering.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/ModuleUtils.h>
#include <Compiler/Compiler.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>


namespace Rift::Compiler::LLVM
{
	llvm::Value* GetLiteralI32(
	    llvm::LLVMContext& llvm, AST::TAccessRef<const CLiteralI32> access, AST::Id id)
	{
		return llvm::ConstantInt::get(
		    llvm, llvm::APInt(32, access.Get<const CLiteralI32>(id).value));
	}

	llvm::Value* GetLiteralFloat(
	    llvm::LLVMContext& llvm, AST::TAccessRef<const CLiteralFloat> access, AST::Id id)
	{
		return llvm::ConstantFP::get(
		    llvm, llvm::APFloat(access.Get<const CLiteralFloat>(id).value));
	}

	void DeclareStruct(
	    llvm::LLVMContext& llvm, AST::TAccessRef<const CType, CIRStruct> access, AST::Id structId)
	{
		ZoneScoped;
		const Name name = access.Get<const CType>(structId).name;
		access.Add(structId, CIRStruct{llvm::StructType::create(llvm, ToLLVM(name))});
	}

	void DefineStruct(
	    llvm::LLVMContext& llvm, AST::TAccessRef<const CIRStruct> access, AST::Id structId)
	{
		ZoneScoped;
		llvm::StructType* irStruct = access.Get<const CIRStruct>(structId).instance;

		TArray<llvm::Type*> memberTypes;
		irStruct->setBody(ToLLVM(memberTypes));
	}

	void GenerateIRModule(
	    Context& context, AST::Id moduleId, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		ZoneScoped;
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

	void CreateEntry(Context& context, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder,
	    const CIRModule& irModule)
	{
		llvm::FunctionType* mainType = llvm::FunctionType::get(builder.getInt32Ty(), false);
		llvm::Function* main         = llvm::Function::Create(
		            mainType, llvm::Function::ExternalLinkage, "main", irModule.instance.Get());
		llvm::BasicBlock* entry = llvm::BasicBlock::Create(llvm, "entry", main);
		builder.SetInsertPoint(entry);

		builder.CreateRet(llvm::ConstantInt::get(llvm, llvm::APInt(32, 0)));
	}

	void GenerateIR(Context& context, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		for (AST::Id moduleId : AST::ListAll<CModule>(context.ast))
		{
			GenerateIRModule(context, moduleId, llvm, builder);
		}

		for (AST::Id moduleId : AST::ListAll<CModule, CIRModule>(context.ast))
		{
			const auto& mod = context.ast.Get<const CModule>(moduleId);
			if (mod.target == ModuleTarget::Executable)
			{
				const auto& irModule = context.ast.Get<const CIRModule>(moduleId);
				LLVM::CreateEntry(context, llvm, builder, irModule);
			}
		}
	}
}    // namespace Rift::Compiler::LLVM
