// Copyright 2015-2022 Piperift - All rights reserved

#include "LLVMBackend/IRGeneration.h"

#include "LLVMBackend/Components/CIRFunction.h"
#include "LLVMBackend/Components/CIRInstruction.h"
#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/Components/CIRType.h"
#include "LLVMBackend/Components/CIRValue.h"
#include "LLVMBackend/LLVMHelpers.h"

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprCall.h>
#include <AST/Components/CExprOutputs.h>
#include <AST/Components/CExprType.h>
#include <AST/Components/CLiteralBool.h>
#include <AST/Components/CLiteralFloating.h>
#include <AST/Components/CLiteralIntegral.h>
#include <AST/Components/CLiteralString.h>
#include <AST/Components/CStmtIf.h>
#include <AST/Components/CStmtOutputs.h>
#include <AST/Components/CStmtReturn.h>
#include <AST/Components/CType.h>
#include <AST/Filtering.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Names.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>


namespace Rift::Compiler::LLVM
{
	using namespace llvm;


	using BlockAccessRef = TAccessRef<CStmtOutput, CStmtOutputs, CStmtIf, CExprCallId, CIRFunction>;

	// Forward declarations
	void AddStmtBlock(Context& context, LLVMContext& llvm, IRBuilder<>& builder,
	    BlockAccessRef access, AST::Id firstStmtId, BasicBlock* block, const CIRFunction& function);
	BasicBlock* AddIf(Context& context, LLVMContext& llvm, IRBuilder<>& builder,
	    BlockAccessRef access, AST::Id id, const CIRFunction& function);
	void AddCall(Context& context, LLVMContext& llvm, IRBuilder<>& builder, const CExprCallId& call,
	    BlockAccessRef access);


	void BindNativeTypes(LLVMContext& llvm, TAccessRef<CType, TWrite<CIRType>> access)
	{
		const auto& nativeTypes = access.GetAST().GetNativeTypes();
		access.Add<CIRType>(nativeTypes.boolId, {llvm::Type::getInt8Ty(llvm)});
		access.Add<CIRType>(nativeTypes.floatId, {llvm::Type::getFloatTy(llvm)});
		access.Add<CIRType>(nativeTypes.doubleId, {llvm::Type::getDoubleTy(llvm)});
		access.Add<CIRType>(nativeTypes.u8Id, {llvm::Type::getInt8Ty(llvm)});
		access.Add<CIRType>(nativeTypes.i8Id, {llvm::Type::getInt8Ty(llvm)});
		access.Add<CIRType>(nativeTypes.u16Id, {llvm::Type::getInt16Ty(llvm)});
		access.Add<CIRType>(nativeTypes.i16Id, {llvm::Type::getInt16Ty(llvm)});
		access.Add<CIRType>(nativeTypes.u32Id, {llvm::Type::getInt32Ty(llvm)});
		access.Add<CIRType>(nativeTypes.i32Id, {llvm::Type::getInt32Ty(llvm)});
		access.Add<CIRType>(nativeTypes.u64Id, {llvm::Type::getInt64Ty(llvm)});
		access.Add<CIRType>(nativeTypes.i64Id, {llvm::Type::getInt64Ty(llvm)});
		// access.Add<CIRType>(nativeTypes.stringId, {});
	}

	void DeclareStructs(LLVMContext& llvm, TAccessRef<CType, TWrite<CIRType>> access,
	    TSpan<AST::Id> ids, bool areClasses)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			const Name name = access.Get<const CType>(id).name;
			access.Add(id, CIRType{StructType::create(llvm, ToLLVM(name))});
		}
	}

	void DefineStructs(Context& context, LLVMContext& llvm,
	    TAccessRef<CIRType, CParent, CIdentifier, CDeclVariable> access, TSpan<AST::Id> ids,
	    bool areClasses)
	{
		ZoneScoped;
		TArray<AST::Id> memberIds;
		TArray<llvm::Type*> memberTypes;
		for (AST::Id id : ids)
		{
			auto* irStruct = static_cast<StructType*>(access.Get<const CIRType>(id).instance);

			// Add members
			memberIds.Empty(false);
			memberTypes.Empty(false);
			AST::Hierarchy::GetChildren(access, id, memberIds);
			AST::RemoveIfNot<CDeclVariable>(access, memberIds);
			for (AST::Id memberId : memberIds)
			{
				const auto& var = access.Get<const CDeclVariable>(memberId);
				if (auto* irType = access.TryGet<const CIRType>(var.typeId))
				{
					memberTypes.Add(irType->instance);
				}
				else
				{
					const Name memberName = Names::GetName(access, memberId);
					const Name typeName   = Names::GetName(access, id);
					context.AddError(Strings::Format(
					    "Variable '{}' in struct '{}' has an invalid type", memberName, typeName));
				}
			}
			irStruct->setBody(ToLLVM(memberTypes));
		}
	}

	void DeclareFunctions(Context& context, LLVMContext& llvm, IRBuilder<>& builder,
	    TAccessRef<TWrite<CIRFunction>, CIdentifier, CExprType, CExprOutputs, CIRType, CParent>
	        access,
	    TSpan<AST::Id> ids, Module& irModule)
	{
		ZoneScoped;
		TArray<AST::Id> inputIds;
		TArray<llvm::Type*> inputTypes;
		for (AST::Id id : ids)
		{
			auto& functionComp = access.Add<CIRFunction>(id);

			// Gather arguments
			AST::Hierarchy::GetChildren(access, id, inputIds);
			AST::RemoveIfNot<CExprOutputs>(access, inputIds);
			AST::RemoveIfNot<CExprType>(access, inputIds);
			for (i32 i = 0; i < inputIds.Size(); ++i)
			{
				AST::Id inputId = inputIds[i];
				AST::Id typeId  = access.Get<const CExprType>(inputId).id;
				if (auto* irType = access.TryGet<const CIRType>(typeId))
				{
					inputTypes.Add(irType->instance);
				}
				else
				{
					const Name argName      = Names::GetName(access, inputId);
					const Name functionName = Names::GetName(access, id);
					context.AddError(Strings::Format(
					    "Input '{}' in function '{}' has an invalid type", argName, functionName));

					inputIds.RemoveAt(i, false);    // Remove input to keep ids stable with args
					--i;
				}
			}

			// Create function
			const CIdentifier& ident = access.Get<const CIdentifier>(id);
			auto* functionType = FunctionType::get(builder.getVoidTy(), ToLLVM(inputTypes), false);
			functionComp.instance = Function::Create(
			    functionType, Function::ExternalLinkage, ToLLVM(ident.name), &irModule);

			// Set argument names
			i32 i            = 0;
			const auto& args = functionComp.instance->args();
			for (auto& arg : args)
			{
				Name name = Names::GetName(access, inputIds[i++]);
				arg.setName(ToLLVM(name));
			}

			// Cache final inputs
			functionComp.inputs   = {args.begin(), args.end()};
			functionComp.inputIds = inputIds;
			inputIds.Empty(false);
			inputTypes.Empty(false);
		}
	}

	void AddStmtBlock(Context& context, LLVMContext& llvm, IRBuilder<>& builder,
	    BlockAccessRef access, AST::Id firstStmtId, BasicBlock* block, const CIRFunction& function)
	{
		ZoneScoped;
		builder.SetInsertPoint(block);

		AST::Id splitId = AST::NoId;
		TArray<AST::Id> stmtIds;
		AST::Statements::GetChain(access, firstStmtId, stmtIds, splitId);

		for (AST::Id id : stmtIds)
		{
			if (const auto* call = access.TryGet<const CExprCallId>(id))
			{
				AddCall(context, llvm, builder, *call, access);
			}
		}

		if (splitId != AST::NoId)
		{
			if (const auto* ifComp = access.TryGet<const CStmtIf>(splitId))
			{
				AddIf(context, llvm, builder, access, splitId, function);
			}
		}
		// TODO: Resolve continuation block and generate it
	}

	BasicBlock* AddIf(Context& context, LLVMContext& llvm, IRBuilder<>& builder,
	    BlockAccessRef access, AST::Id id, const CIRFunction& function)
	{
		const auto& outputs      = access.Get<const CStmtOutputs>(id);
		const auto& connectedIds = outputs.linkInputNodes;
		Check(connectedIds.Size() == 2);

		// Temporarily using value false until we have expressions
		Value* condV = ConstantInt::get(llvm, APInt(8, false, true));

		// Convert condition to a bool by comparing non-equal to 0.0.
		condV = builder.CreateFCmpONE(condV, ConstantFP::get(llvm, APFloat(0.0)), "ifcond");

		BasicBlock* thenBlock = BasicBlock::Create(llvm, "then");
		BasicBlock* elseBlock = BasicBlock::Create(llvm, "else");
		BasicBlock* contBlock = BasicBlock::Create(llvm, "continue");
		builder.CreateCondBr(condV, thenBlock, elseBlock);

		function.instance->getBasicBlockList().push_back(thenBlock);
		AddStmtBlock(context, llvm, builder, access, connectedIds[0], thenBlock, function);
		builder.CreateBr(contBlock);

		function.instance->getBasicBlockList().push_back(elseBlock);
		AddStmtBlock(context, llvm, builder, access, connectedIds[1], elseBlock, function);
		builder.CreateBr(contBlock);

		function.instance->getBasicBlockList().push_back(contBlock);
		return contBlock;
	}

	void AddCall(Context& context, LLVMContext& llvm, IRBuilder<>& builder, const CExprCallId& call,
	    BlockAccessRef access)
	{
		const AST::Id functionId = call.functionId;
		if (!access.IsValid(functionId))
		{
			context.AddError("Call to an unknown function");
			return;
		}
		const auto* function = access.TryGet<const CIRFunction>(functionId);
		if (!Ensure(function))
		{
			context.AddError("Call to an invalid function");
			return;
		}

		TArray<Value*> args;
		// TODO: Pass arguments
		builder.CreateCall(function->instance, ToLLVM(args), "calltmp");
	}

	void DefineFunctions(Context& context, LLVMContext& llvm, IRBuilder<>& builder,
	    BlockAccessRef access, TSpan<AST::Id> ids, Module& irModule)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			const auto& irFunction = access.Get<const CIRFunction>(id);
			BasicBlock* block      = BasicBlock::Create(llvm, "entry", irFunction.instance);
			AddStmtBlock(context, llvm, builder, access, id, block, irFunction);

			verifyFunction(*irFunction.instance);
		}
	}


	void CreateMain(
	    Context& context, LLVMContext& llvm, IRBuilder<>& builder, const CIRModule& irModule)
	{
		FunctionType* mainType = FunctionType::get(builder.getInt32Ty(), false);
		Function* main =
		    Function::Create(mainType, Function::ExternalLinkage, "main", irModule.instance.Get());
		BasicBlock* entry = BasicBlock::Create(llvm, "entry", main);
		builder.SetInsertPoint(entry);

		builder.CreateRet(ConstantInt::get(llvm, APInt(32, 0)));
	}

	void GenerateLiterals(LLVMContext& llvm, TAccessRef<CLiteralBool, CLiteralIntegral,
	                                             CLiteralFloating, CLiteralString, TWrite<CIRValue>>
	                                             access)
	{
		for (AST::Id id : AST::ListAll<CLiteralBool>(access))
		{
			const auto& boolean = access.Get<const CLiteralBool>(id);
			Value* value        = ConstantInt::get(llvm, APInt(8, boolean.value, true));
			access.Add<CIRValue>(id, value);
		}
		for (AST::Id id : AST::ListAll<CLiteralIntegral>(access))
		{
			const auto& integral = access.Get<const CLiteralIntegral>(id);
			Value* value         = ConstantInt::get(
			            llvm, APInt(integral.GetSize(), integral.value, integral.IsSigned()));
			access.Add<CIRValue>(id, value);
		}
		for (AST::Id id : AST::ListAll<CLiteralFloating>(access))
		{
			const auto& floating = access.Get<const CLiteralFloating>(id);
			Value* value         = ConstantFP::get(llvm,
			            APFloat(floating.type == FloatingType::F32 ? static_cast<float>(floating.value)
			                                                       : floating.value));
			access.Add<CIRValue>(id, value);
		}
		for (AST::Id id : AST::ListAll<CLiteralString>(access))
		{
			const auto& string = access.Get<const CLiteralString>(id);
			Value* value       = ConstantDataArray::getString(llvm, ToLLVM(string.value));
			access.Add<CIRValue>(id, value);
		}
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

		DeclareStructs(llvm, ast, structIds, false);
		DeclareStructs(llvm, ast, classIds, true);    // Declare classes

		TArray<AST::Id> functionIds;
		AST::Hierarchy::GetChildren(ast, typeIds, functionIds);
		AST::RemoveIfNot<CDeclFunction>(ast, functionIds);
		DeclareFunctions(context, llvm, builder, ast, functionIds, irModule);

		DefineStructs(context, llvm, ast, structIds, false);
		DefineStructs(context, llvm, ast, classIds, true);    // Define classes
		DefineFunctions(context, llvm, builder, ast, functionIds, irModule);
	}

	void GenerateIR(Context& context, LLVMContext& llvm, IRBuilder<>& builder)
	{
		BindNativeTypes(llvm, context.ast);
		GenerateLiterals(llvm, context.ast);

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
				LLVM::CreateMain(context, llvm, builder, irModule);
			}
		}
	}
}    // namespace Rift::Compiler::LLVM
