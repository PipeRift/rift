// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend/IRGeneration.h"

#include "LLVMBackend/Components/CIRFunction.h"
#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/Components/CIRStruct.h"
#include "LLVMBackend/LLVMHelpers.h"

#include <AST/Components/CClassDecl.h>
#include <AST/Components/CFunctionDecl.h>
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
	using namespace llvm;

	Value* GetLiteralI32(LLVMContext& llvm, AST::TAccessRef<const CLiteralI32> access, AST::Id id)
	{
		return ConstantInt::get(llvm, APInt(32, access.Get<const CLiteralI32>(id).value));
	}

	Value* GetLiteralFloat(
	    LLVMContext& llvm, AST::TAccessRef<const CLiteralFloat> access, AST::Id id)
	{
		return ConstantFP::get(llvm, APFloat(access.Get<const CLiteralFloat>(id).value));
	}

	void DeclareStruct(
	    LLVMContext& llvm, AST::TAccessRef<const CType, CIRStruct> access, AST::Id id)
	{
		ZoneScoped;
		const Name name = access.Get<const CType>(id).name;
		access.Add(id, CIRStruct{StructType::create(llvm, ToLLVM(name))});
	}

	void DefineStruct(LLVMContext& llvm, AST::TAccessRef<const CIRStruct> access, AST::Id id)
	{
		ZoneScoped;
		StructType* irStruct = access.Get<const CIRStruct>(id).instance;

		TArray<Type*> memberTypes;
		irStruct->setBody(ToLLVM(memberTypes));
	}

	void DeclareFunction(LLVMContext& llvm, IRBuilder<>& builder,
	    AST::TAccessRef<CIRFunction, const CIdentifier> access, AST::Id id, Module& irModule)
	{
		ZoneScoped;

		const CIdentifier& ident = access.Get<const CIdentifier>(id);

		FunctionType* type = FunctionType::get(builder.getVoidTy(), false);

		access.Add(id, CIRFunction{Function::Create(
		                   type, Function::ExternalLinkage, ToLLVM(ident.name), &irModule)});
	}

	void CreateEntry(
	    Context& context, LLVMContext& llvm, IRBuilder<>& builder, const CIRModule& irModule)
	{
		FunctionType* mainType = FunctionType::get(builder.getInt32Ty(), false);
		Function* main =
		    Function::Create(mainType, Function::ExternalLinkage, "main", irModule.instance.Get());
		BasicBlock* entry = BasicBlock::Create(llvm, "entry", main);
		builder.SetInsertPoint(entry);

		builder.CreateRet(ConstantInt::get(llvm, APInt(32, 0)));
	}

	void GenerateIRModule(
	    Context& context, AST::Id moduleId, LLVMContext& llvm, IRBuilder<>& builder)
	{
		ZoneScoped;
		const auto& config = context.config;
		auto& ast          = context.ast;

		const Name name = Modules::GetModuleName(context.ast, moduleId);

		CIRModule& module = context.ast.Add<CIRModule>(moduleId);
		module.instance   = Move(MakeOwned<Module>(ToLLVM(name), llvm));
		Module& irModule  = *module.instance.Get();

		TArray<AST::Id> types;
		AST::Hierarchy::GetChildren(ast, moduleId, types);
		AST::RemoveIfNot<CType>(ast, types);
		TArray<AST::Id> classes = types, structs = types;

		AST::RemoveIfNot<CClassDecl>(ast, classes);
		AST::RemoveIfNot<CStructDecl>(ast, structs);

		// Declare types
		AST::TAccess<const CType, CIRStruct> declareStructAccess{ast};
		for (AST::Id id : structs)
		{
			DeclareStruct(llvm, declareStructAccess, id);
		}
		for (AST::Id id : classes)
		{
			DeclareStruct(llvm, declareStructAccess, id);
		}

		// Declare functions
		TArray<AST::Id> functions;
		AST::Hierarchy::GetChildren(ast, types, functions);
		AST::RemoveIfNot<CFunctionDecl>(ast, functions);
		AST::TAccess<CIRFunction, const CIdentifier> declareFunctionAccess{ast};
		for (AST::Id id : functions)
		{
			DeclareFunction(llvm, builder, declareFunctionAccess, id, irModule);
		}

		// Define types
		AST::TAccess<const CIRStruct> defineStructAccess{ast};
		for (AST::Id id : structs)
		{
			DefineStruct(llvm, defineStructAccess, id);
		}
		for (AST::Id id : classes)
		{
			DefineStruct(llvm, defineStructAccess, id);
		}
	}

	void GenerateIR(Context& context, LLVMContext& llvm, IRBuilder<>& builder)
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
