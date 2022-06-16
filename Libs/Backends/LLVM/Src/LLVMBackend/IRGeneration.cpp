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
#include <AST/Components/CDeclFunctionInterface.h>
#include <AST/Components/CDeclFunctionLibrary.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprCall.h>
#include <AST/Components/CExprInputs.h>
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
#include <AST/Components/Tags/CInvalid.h>
#include <AST/Id.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Names.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <Pipe/ECS/Filtering.h>


namespace rift::Compiler::LLVM
{
	using BlockAccessRef = TAccessRef<CStmtOutput, CStmtOutputs, CExprInputs, CStmtIf, CExprCallId,
	    CIRFunction, CIRValue>;

	// Forward declarations
	void AddStmtBlock(ModuleIRGen& gen, BlockAccessRef access, AST::Id firstStmtId,
	    llvm::BasicBlock* block, const CIRFunction& function);
	llvm::BasicBlock* AddIf(
	    ModuleIRGen& gen, BlockAccessRef access, AST::Id id, const CIRFunction& function);
	void AddCall(ModuleIRGen& gen, AST::Id id, const CExprCallId& call, BlockAccessRef access);


	void BindNativeTypes(llvm::LLVMContext& llvm, TAccessRef<CType, TWrite<CIRType>> access)
	{
		const auto& nativeTypes = static_cast<AST::Tree&>(access.GetContext()).GetNativeTypes();
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

	void DeclareStructs(
	    ModuleIRGen& gen, TAccessRef<CType, TWrite<CIRType>> access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			const Name name = access.Get<const CType>(id).name;
			access.Add(id, CIRType{llvm::StructType::create(gen.llvm, ToLLVM(name))});
		}
	}

	void DefineStructs(ModuleIRGen& gen,
	    TAccessRef<CIRType, CParent, CIdentifier, CDeclVariable> access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		TArray<AST::Id> memberIds;
		TArray<llvm::Type*> memberTypes;
		for (AST::Id id : ids)
		{
			auto* irStruct = static_cast<llvm::StructType*>(access.Get<const CIRType>(id).instance);

			// Add members
			memberIds.Empty(false);
			memberTypes.Empty(false);
			AST::Hierarchy::GetChildren(access, id, memberIds);
			ecs::ExcludeIfNot<CDeclVariable>(access, memberIds);
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
					gen.compiler.AddError(Strings::Format(
					    "Variable '{}' in struct '{}' has an invalid type", memberName, typeName));
				}
			}
			irStruct->setBody(ToLLVM(memberTypes));
		}
	}

	using DeclareFunctionAccess = TAccessRef<TWrite<CIRFunction>, CIdentifier, CExprType,
	    CExprOutputs, CIRType, CParent, CInvalid>;
	void DeclareFunctions(ModuleIRGen& gen, DeclareFunctionAccess access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		TArray<AST::Id> inputIds;
		TArray<llvm::Type*> inputTypes;
		for (AST::Id id : ids)
		{
			auto& functionComp = access.Add<CIRFunction>(id);

			inputIds.Empty(false);
			inputTypes.Empty(false);
			if (auto* outputs = access.TryGet<const CExprOutputs>(id))
			{
				for (i32 i = 0; i < outputs->pinIds.Size(); ++i)
				{
					AST::Id inputId = outputs->pinIds[i];
					if (access.Has<CInvalid>(inputId))
					{
						continue;
					}

					inputIds.Add(inputId);

					AST::Id typeId = access.Get<const CExprType>(inputId).id;
					auto* irType   = access.TryGet<const CIRType>(typeId);
					if (irType && irType->instance)
					{
						inputTypes.Add(irType->instance);
					}
					else
					{
						const Name argName      = Names::GetName(access, inputId);
						const Name functionName = Names::GetName(access, id);
						gen.compiler.AddError(Strings::Format(
						    "Input '{}' in function '{}' has an invalid type. Using i32 instead.",
						    argName, functionName));
						inputTypes.Add(gen.builder.getInt32Ty());
					}
				}
			}

			// Create function
			auto& ident = access.Get<const CIdentifier>(id);
			auto* functionType =
			    llvm::FunctionType::get(gen.builder.getVoidTy(), ToLLVM(inputTypes), false);
			functionComp.instance = llvm::Function::Create(
			    functionType, llvm::Function::ExternalLinkage, ToLLVM(ident.name), &gen.module);

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

	void AddStmtBlock(ModuleIRGen& gen, BlockAccessRef access, AST::Id firstStmtId,
	    llvm::BasicBlock* block, const CIRFunction& function)
	{
		ZoneScoped;
		gen.builder.SetInsertPoint(block);

		AST::Id splitId = AST::NoId;
		TArray<AST::Id> stmtIds;
		AST::Statements::GetChain(access, firstStmtId, stmtIds, splitId);

		for (AST::Id id : stmtIds)
		{
			if (const auto* call = access.TryGet<const CExprCallId>(id))
			{
				AddCall(gen, id, *call, access);
			}
		}

		if (splitId != AST::NoId)
		{
			if (access.Has<const CStmtIf>(splitId))
			{
				AddIf(gen, access, splitId, function);
			}
		}
		// TODO: Resolve continuation block and generate it
	}

	llvm::Value* AddExpr(ModuleIRGen& gen, BlockAccessRef access, const OutputId& output)
	{
		const auto* value =
		    !IsNone(output.pinId) ? access.TryGet<const CIRValue>(output.pinId) : nullptr;
		if (value)
		{
			return value->instance;
		}
		return nullptr;
	}

	llvm::BasicBlock* AddIf(
	    ModuleIRGen& gen, BlockAccessRef access, AST::Id id, const CIRFunction& function)
	{
		const auto& outputs      = access.Get<const CStmtOutputs>(id);
		const auto& connectedIds = outputs.linkInputNodes;
		Check(connectedIds.Size() == 2);
		const auto& exprInputs = access.Get<const CExprInputs>(id);
		Check(exprInputs.linkedOutputs.Size() == 1);

		llvm::Value* condV = AddExpr(gen, access, exprInputs.linkedOutputs.First());
		if (!condV)
		{
			// Assign false by default
			condV = llvm::ConstantInt::get(gen.llvm, llvm::APInt(1, false, true));
		}

		auto* thenBlock = llvm::BasicBlock::Create(gen.llvm, "then");
		auto* elseBlock = llvm::BasicBlock::Create(gen.llvm, "else");
		auto* contBlock = llvm::BasicBlock::Create(gen.llvm, "continue");
		gen.builder.CreateCondBr(condV, thenBlock, elseBlock);

		function.instance->getBasicBlockList().push_back(thenBlock);
		AddStmtBlock(gen, access, connectedIds[0], thenBlock, function);
		gen.builder.CreateBr(contBlock);

		function.instance->getBasicBlockList().push_back(elseBlock);
		AddStmtBlock(gen, access, connectedIds[1], elseBlock, function);
		gen.builder.CreateBr(contBlock);

		function.instance->getBasicBlockList().push_back(contBlock);
		return contBlock;
	}

	void AddCall(ModuleIRGen& gen, AST::Id id, const CExprCallId& call, BlockAccessRef access)
	{
		const AST::Id functionId = call.functionId;
		if (!access.IsValid(functionId))
		{
			gen.compiler.AddError("Call to an unknown function");
			return;
		}
		const auto* function = access.TryGet<const CIRFunction>(functionId);
		if (!Ensure(function))
		{
			gen.compiler.AddError("Call to an invalid function");
			return;
		}

		TArray<llvm::Value*> args;
		if (auto* inputs = access.TryGet<const CExprInputs>(id))
		{
			args.Reserve(inputs->linkedOutputs.Size());
			for (i32 i = 0; i < inputs->linkedOutputs.Size(); ++i)
			{
				OutputId output = inputs->linkedOutputs[i];
				if (!output.IsNone())
				{
					args.Add(AddExpr(gen, access, output));
				}
			}
		}
		gen.builder.CreateCall(function->instance, ToLLVM(args));
	}

	void DefineFunctions(ModuleIRGen& gen, BlockAccessRef access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			const auto& irFunction = access.Get<const CIRFunction>(id);
			auto* block = llvm::BasicBlock::Create(gen.llvm, "entry", irFunction.instance);

			const auto& output = access.Get<const CStmtOutput>(id);
			AddStmtBlock(gen, access, output.linkInputNode, block, irFunction);

			verifyFunction(*irFunction.instance);
		}
	}


	void CreateMain(ModuleIRGen& gen)
	{
		auto* mainType = llvm::FunctionType::get(gen.builder.getInt32Ty(), false);
		auto* main =
		    llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", &gen.module);
		auto* entry = llvm::BasicBlock::Create(gen.llvm, "entry", main);
		gen.builder.SetInsertPoint(entry);

		gen.builder.CreateRet(llvm::ConstantInt::get(gen.llvm, llvm::APInt(32, 0)));
	}

	using GenerateLiteralsAccs = TAccessRef<CLiteralBool, CLiteralIntegral, CLiteralFloating,
	    CLiteralString, TWrite<CIRValue>>;
	void GenerateLiterals(llvm::LLVMContext& llvm, GenerateLiteralsAccs access)
	{
		for (AST::Id id : ecs::ListAll<CLiteralBool>(access))
		{
			const auto& boolean = access.Get<const CLiteralBool>(id);
			llvm::Value* value  = llvm::ConstantInt::get(llvm, llvm::APInt(1, boolean.value, true));
			access.Add<CIRValue>(id, value);
		}
		for (AST::Id id : ecs::ListAll<CLiteralIntegral>(access))
		{
			const auto& integral = access.Get<const CLiteralIntegral>(id);
			llvm::Value* value   = llvm::ConstantInt::get(
			      llvm, llvm::APInt(integral.GetSize(), integral.value, integral.IsSigned()));
			access.Add<CIRValue>(id, value);
		}
		for (AST::Id id : ecs::ListAll<CLiteralFloating>(access))
		{
			const auto& floating = access.Get<const CLiteralFloating>(id);
			llvm::Value* value =
			    llvm::ConstantFP::get(llvm, llvm::APFloat(floating.type == FloatingType::F32
			                                                  ? static_cast<float>(floating.value)
			                                                  : floating.value));
			access.Add<CIRValue>(id, value);
		}
		for (AST::Id id : ecs::ListAll<CLiteralString>(access))
		{
			const auto& string = access.Get<const CLiteralString>(id);
			llvm::Value* value = llvm::ConstantDataArray::getString(llvm, ToLLVM(string.value));
			access.Add<CIRValue>(id, value);
		}
	}

	void GenerateIRModule(
	    Context& context, AST::Id moduleId, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		ZoneScoped;
		auto& ast = context.ast;

		const Name name = Modules::GetModuleName(context.ast, moduleId);

		CIRModule& module      = context.ast.Add<CIRModule>(moduleId);
		module.instance        = Move(MakeOwned<llvm::Module>(ToLLVM(name), llvm));
		llvm::Module& irModule = *module.instance.Get();

		ModuleIRGen gen{context, irModule, llvm, builder};

		// Filter all module rift types
		TArray<AST::Id> typeIds;
		AST::Hierarchy::GetChildren(ast, moduleId, typeIds);
		ecs::ExcludeIfNot<CType>(ast, typeIds);
		TArray<AST::Id> structIds            = ecs::GetIf<CDeclStruct>(ast, typeIds);
		TArray<AST::Id> classIds             = ecs::GetIf<CDeclClass>(ast, typeIds);
		TArray<AST::Id> functionLibraryIds   = ecs::GetIf<CDeclFunctionLibrary>(ast, typeIds);
		TArray<AST::Id> functionInterfaceIds = ecs::GetIf<CDeclFunctionInterface>(ast, typeIds);

		DeclareStructs(gen, ast, structIds);
		DeclareStructs(gen, ast, classIds);

		TArray<AST::Id> functionIds;
		AST::Hierarchy::GetChildren(ast, classIds, functionIds);
		AST::Hierarchy::GetChildren(ast, functionLibraryIds, functionIds);
		AST::Hierarchy::GetChildren(ast, functionInterfaceIds, functionIds);
		ecs::ExcludeIfNot<CDeclFunction>(ast, functionIds);
		DeclareFunctions(gen, ast, functionIds);

		DefineStructs(gen, ast, structIds);
		DefineStructs(gen, ast, classIds);

		DefineFunctions(gen, ast, functionIds);

		const auto& mod = context.ast.Get<const CModule>(moduleId);
		if (mod.target == ModuleTarget::Executable)
		{
			CreateMain(gen);
		}
	}

	void GenerateIR(Context& context, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		BindNativeTypes(llvm, context.ast);
		GenerateLiterals(llvm, context.ast);

		for (AST::Id moduleId : ecs::ListAll<CModule>(context.ast))
		{
			GenerateIRModule(context, moduleId, llvm, builder);
		}
	}
}    // namespace rift::Compiler::LLVM
