// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend/IRGeneration.h"

#include "LLVMBackend/Components/CIRFunction.h"
#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/Components/CIRStruct.h"
#include "LLVMBackend/LLVMHelpers.h"

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CLiteralFloating.h>
#include <AST/Components/CLiteralIntegral.h>
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

	Value* GetLiteralIntegral(LLVMContext& llvm, TAccessRef<CLiteralIntegral> access, AST::Id id)
	{
		const auto& integral = access.Get<const CLiteralIntegral>(id);
		return ConstantInt::get(
		    llvm, APInt(integral.GetSize(), integral.value, integral.IsSigned()));
	}

	Value* GetLiteralFloating(LLVMContext& llvm, TAccessRef<CLiteralFloating> access, AST::Id id)
	{
		const auto& floating = access.Get<const CLiteralFloating>(id);
		return ConstantFP::get(
		    llvm, APFloat(floating.type == FloatingType::F32 ? static_cast<float>(floating.value)
		                                                     : floating.value));
	}

	void DeclareStructs(
	    LLVMContext& llvm, TAccessRef<CType, TWrite<CIRStruct>> access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			const Name name = access.Get<const CType>(id).name;
			access.Add(id, CIRStruct{StructType::create(llvm, ToLLVM(name))});
		}
	}

	void DeclareClasses(
	    LLVMContext& llvm, TAccessRef<CType, TWrite<CIRStruct>> access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			const Name name = access.Get<const CType>(id).name;
			access.Add(id, CIRStruct{StructType::create(llvm, ToLLVM(name))});
		}
	}

	void DefineStructs(LLVMContext& llvm, TAccessRef<CIRStruct> access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			StructType* irStruct = access.Get<const CIRStruct>(id).instance;

			TArray<llvm::Type*> memberTypes;
			irStruct->setBody(ToLLVM(memberTypes));
		}
	}

	void DefineClasses(LLVMContext& llvm, TAccessRef<CIRStruct> access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			StructType* irStruct = access.Get<const CIRStruct>(id).instance;

			TArray<llvm::Type*> memberTypes;
			irStruct->setBody(ToLLVM(memberTypes));
		}
	}

	void DeclareFunctions(LLVMContext& llvm, IRBuilder<>& builder,
	    TAccessRef<TWrite<CIRFunction>, CIdentifier> access, TSpan<AST::Id> ids, Module& irModule)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			const CIdentifier& ident = access.Get<const CIdentifier>(id);

			FunctionType* type = FunctionType::get(builder.getVoidTy(), false);
			access.Add<CIRFunction>(id,
			    {Function::Create(type, Function::ExternalLinkage, ToLLVM(ident.name), &irModule)});
		}
	}

	void DefineFunctions(LLVMContext& llvm, IRBuilder<>& builder, TAccessRef<CIRFunction> access,
	    TSpan<AST::Id> functionIds, Module& irModule)
	{
		ZoneScoped;
		for (AST::Id id : functionIds) {}
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
		auto& ast = context.ast;

		const Name name = Modules::GetModuleName(context.ast, moduleId);

		CIRModule& module = context.ast.Add<CIRModule>(moduleId);
		module.instance   = Move(MakeOwned<Module>(ToLLVM(name), llvm));
		Module& irModule  = *module.instance.Get();

		// Filter all classIds and structIds
		TArray<AST::Id> typeIds;
		AST::Hierarchy::GetChildren(ast, moduleId, typeIds);
		AST::RemoveIfNot<CType>(ast, typeIds);
		TArray<AST::Id> classIds = typeIds, structIds = typeIds;
		AST::RemoveIfNot<CDeclStruct>(ast, structIds);
		AST::RemoveIfNot<CDeclClass>(ast, classIds);

		// Filter functions
		TArray<AST::Id> functionIds;
		AST::Hierarchy::GetChildren(ast, typeIds, functionIds);
		AST::RemoveIfNot<CDeclFunction>(ast, functionIds);

		DeclareStructs(llvm, ast, structIds);
		DeclareClasses(llvm, ast, classIds);
		DeclareFunctions(llvm, builder, ast, functionIds, irModule);

		DefineStructs(llvm, ast, structIds);
		DefineClasses(llvm, ast, classIds);
		DefineFunctions(llvm, builder, ast, functionIds, irModule);
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
