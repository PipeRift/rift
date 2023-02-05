// Copyright 2015-2023 Piperift - All rights reserved

#include "LLVMBackend/IRGeneration.h"

#include "LLVMBackend/Components/CIRFunction.h"
#include "LLVMBackend/Components/CIRModule.h"
#include "LLVMBackend/Components/CIRType.h"
#include "LLVMBackend/Components/CIRValue.h"
#include "LLVMBackend/LLVMHelpers.h"

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
#include <AST/Components/CRiftModule.h>
#include <AST/Components/CStmtIf.h>
#include <AST/Components/CStmtOutputs.h>
#include <AST/Components/CStmtReturn.h>
#include <AST/Components/Tags/CInvalid.h>
#include <AST/Id.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <Pipe/ECS/Filtering.h>
#include <Pipe/ECS/Utils/Hierarchy.h>


namespace rift::compiler::LLVM
{
	using BlockAccessRef = TAccessRef<AST::CStmtOutput, AST::CStmtOutputs, AST::CExprInputs,
	    AST::CStmtIf, AST::CExprCallId, CIRFunction, CIRValue, AST::CNamespace>;

	// Forward declarations
	void AddStmtBlock(ModuleIRGen& gen, BlockAccessRef access, AST::Id firstStmtId,
	    llvm::BasicBlock* block, const CIRFunction& function);
	llvm::BasicBlock* AddIf(
	    ModuleIRGen& gen, BlockAccessRef access, AST::Id id, const CIRFunction& function);
	void AddCall(ModuleIRGen& gen, AST::Id id, const AST::CExprCallId& call, BlockAccessRef access);


	void BindNativeTypes(
	    llvm::LLVMContext& llvm, TAccessRef<AST::CDeclType, TWrite<CIRType>> access)
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

	void DeclareStructs(ModuleIRGen& gen,
	    TAccessRef<AST::CDeclType, AST::CNamespace, TWrite<CIRType>, AST::CChild, AST::CModule>
	        access,
	    TSpan<AST::Id> ids)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			p::String name = AST::GetFullName(access, id);
			access.Add(id, CIRType{llvm::StructType::create(gen.llvm, ToLLVM(name))});
		}
	}

	void DefineStructs(ModuleIRGen& gen,
	    TAccessRef<CIRType, AST::CParent, AST::CNamespace, AST::CDeclVariable> access,
	    TSpan<AST::Id> ids)
	{
		ZoneScoped;
		TArray<AST::Id> memberIds;
		TArray<llvm::Type*> memberTypes;
		for (AST::Id id : ids)
		{
			auto* irStruct = static_cast<llvm::StructType*>(access.Get<const CIRType>(id).instance);

			// Add members
			memberIds.Clear(false);
			memberTypes.Clear(false);
			p::ecs::GetChildren(access, id, memberIds);
			ecs::ExcludeIfNot<AST::CDeclVariable>(access, memberIds);
			for (AST::Id memberId : memberIds)
			{
				const auto& var = access.Get<const AST::CDeclVariable>(memberId);
				if (auto* irType = access.TryGet<const CIRType>(var.typeId))
				{
					memberTypes.Add(irType->instance);
				}
				else
				{
					const Name memberName = AST::GetName(access, memberId);
					const Name typeName   = AST::GetName(access, id);
					gen.compiler.AddError(Strings::Format(
					    "Variable '{}' in struct '{}' has an invalid type", memberName, typeName));
				}
			}
			irStruct->setBody(ToLLVM(memberTypes));
		}
	}

	using DeclareFunctionAccess = TAccessRef<TWrite<CIRFunction>, AST::CNamespace, AST::CExprTypeId,
	    AST::CExprOutputs, CIRType, AST::CParent, AST::CChild, AST::CInvalid, AST::CModule>;
	void DeclareFunctions(ModuleIRGen& gen, DeclareFunctionAccess access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		TArray<AST::Id> inputIds;
		TArray<llvm::Type*> inputTypes;
		for (AST::Id id : ids)
		{
			auto& functionComp = access.Add<CIRFunction>(id);

			inputIds.Clear(false);
			inputTypes.Clear(false);
			if (auto* outputs = access.TryGet<const AST::CExprOutputs>(id))
			{
				for (i32 i = 0; i < outputs->pinIds.Size(); ++i)
				{
					AST::Id inputId = outputs->pinIds[i];
					if (access.Has<AST::CInvalid>(inputId))
					{
						continue;
					}

					inputIds.Add(inputId);

					AST::Id typeId = access.Get<const AST::CExprTypeId>(inputId).id;
					auto* irType   = access.TryGet<const CIRType>(typeId);
					if (irType && irType->instance)
					{
						inputTypes.Add(irType->instance);
					}
					else
					{
						const Name argName        = AST::GetName(access, inputId);
						const String functionName = AST::GetFullName(access, id);
						gen.compiler.AddError(Strings::Format(
						    "Input '{}' in function '{}' has an invalid type. Using i32 instead.",
						    argName, functionName));
						inputTypes.Add(gen.builder.getInt32Ty());
					}
				}
			}

			// Create function
			p::String name = AST::GetFullName(access, id);
			auto* functionType =
			    llvm::FunctionType::get(gen.builder.getVoidTy(), ToLLVM(inputTypes), false);
			functionComp.instance = llvm::Function::Create(
			    functionType, llvm::Function::ExternalLinkage, ToLLVM(name), &gen.module);

			// Set argument names
			i32 i            = 0;
			const auto& args = functionComp.instance->args();
			for (auto& arg : args)
			{
				Name name = AST::GetName(access, inputIds[i++]);
				arg.setName(ToLLVM(name));
			}

			// Cache final inputs
			functionComp.inputs   = {args.begin(), args.end()};
			functionComp.inputIds = inputIds;
			inputIds.Clear(false);
			inputTypes.Clear(false);
		}
	}

	void AddStmtBlock(ModuleIRGen& gen, BlockAccessRef access, AST::Id firstStmtId,
	    llvm::BasicBlock* block, const CIRFunction& function)
	{
		ZoneScoped;
		gen.builder.SetInsertPoint(block);

		AST::Id splitId = AST::NoId;
		TArray<AST::Id> stmtIds;
		AST::GetStmtChain(access, firstStmtId, stmtIds, splitId);

		for (AST::Id id : stmtIds)
		{
			if (const auto* call = access.TryGet<const AST::CExprCallId>(id))
			{
				AddCall(gen, id, *call, access);
			}
		}

		if (splitId != AST::NoId)
		{
			if (access.Has<const AST::CStmtIf>(splitId))
			{
				AddIf(gen, access, splitId, function);
			}
		}
		// TODO: Resolve continuation block and generate it
	}

	llvm::Value* AddExpr(ModuleIRGen& gen, BlockAccessRef access, const AST::ExprOutput& output)
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
		const auto& outputs      = access.Get<const AST::CStmtOutputs>(id);
		const auto& connectedIds = outputs.linkInputNodes;
		Check(connectedIds.Size() == 2);
		const auto& exprInputs = access.Get<const AST::CExprInputs>(id);
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

	void AddCall(ModuleIRGen& gen, AST::Id id, const AST::CExprCallId& call, BlockAccessRef access)
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
			gen.compiler.AddError(Strings::Format(
			    "Call to an invalid function: '{}'", AST::GetName(access, functionId)));
			return;
		}

		TArray<llvm::Value*> args;
		if (auto* inputs = access.TryGet<const AST::CExprInputs>(id))
		{
			args.Reserve(inputs->linkedOutputs.Size());
			for (i32 i = 0; i < inputs->linkedOutputs.Size(); ++i)
			{
				AST::ExprOutput output = inputs->linkedOutputs[i];
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

			const auto& output = access.Get<const AST::CStmtOutput>(id);
			AddStmtBlock(gen, access, output.linkInputNode, block, irFunction);

			// Generate default return
			gen.builder.CreateRet(nullptr);

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

	using GenerateLiteralsAccs = TAccessRef<AST::CLiteralBool, AST::CLiteralIntegral,
	    AST::CLiteralFloating, AST::CLiteralString, TWrite<CIRValue>>;
	void GenerateLiterals(llvm::LLVMContext& llvm, GenerateLiteralsAccs access)
	{
		for (AST::Id id : ecs::ListAll<AST::CLiteralBool>(access))
		{
			const auto& boolean = access.Get<const AST::CLiteralBool>(id);
			llvm::Value* value  = llvm::ConstantInt::get(llvm, llvm::APInt(1, boolean.value, true));
			access.Add<CIRValue>(id, value);
		}
		for (AST::Id id : ecs::ListAll<AST::CLiteralIntegral>(access))
		{
			const auto& integral = access.Get<const AST::CLiteralIntegral>(id);
			llvm::Value* value   = llvm::ConstantInt::get(
			      llvm, llvm::APInt(integral.GetSize(), integral.value, integral.IsSigned()));
			access.Add<CIRValue>(id, value);
		}
		for (AST::Id id : ecs::ListAll<AST::CLiteralFloating>(access))
		{
			const auto& floating = access.Get<const AST::CLiteralFloating>(id);
			llvm::Value* value =
			    llvm::ConstantFP::get(llvm, llvm::APFloat(floating.type == AST::FloatingType::F32
			                                                  ? static_cast<float>(floating.value)
			                                                  : floating.value));
			access.Add<CIRValue>(id, value);
		}
		for (AST::Id id : ecs::ListAll<AST::CLiteralString>(access))
		{
			const auto& string = access.Get<const AST::CLiteralString>(id);
			llvm::Value* value = llvm::ConstantDataArray::getString(llvm, ToLLVM(string.value));
			access.Add<CIRValue>(id, value);
		}
	}

	void GenerateIRModule(
	    Compiler& compiler, AST::Id moduleId, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		ZoneScoped;
		auto& ast = compiler.ast;

		const Name name = AST::GetModuleName(compiler.ast, moduleId);

		CIRModule& module      = compiler.ast.Add<CIRModule>(moduleId);
		module.instance        = MakeOwned<llvm::Module>(ToLLVM(name), llvm);
		llvm::Module& irModule = *module.instance.Get();

		ModuleIRGen gen{compiler, irModule, llvm, builder};

		// Filter all module rift types
		TArray<AST::Id> typeIds;
		ecs::GetChildren(ast, moduleId, typeIds);
		ecs::ExcludeIfNot<AST::CDeclType>(ast, typeIds);
		TArray<AST::Id> structIds = ecs::GetIf<AST::CDeclStruct>(ast, typeIds);
		TArray<AST::Id> classIds  = ecs::GetIf<AST::CDeclClass>(ast, typeIds);
		TArray<AST::Id> staticIds = ecs::GetIf<AST::CDeclStatic>(ast, typeIds);

		DeclareStructs(gen, ast, structIds);
		DeclareStructs(gen, ast, classIds);

		TArray<AST::Id> functionIds;
		p::ecs::GetChildren(ast, classIds, functionIds);
		p::ecs::GetChildren(ast, staticIds, functionIds);
		ecs::ExcludeIfNot<AST::CDeclFunction>(ast, functionIds);
		DeclareFunctions(gen, ast, functionIds);

		DefineStructs(gen, ast, structIds);
		DefineStructs(gen, ast, classIds);

		DefineFunctions(gen, ast, functionIds);

		const auto* riftMod = compiler.ast.TryGet<const AST::CRiftModule>(moduleId);
		if (riftMod && riftMod->target == AST::RiftModuleTarget::Executable)
		{
			CreateMain(gen);
		}
	}

	void GenerateIR(Compiler& compiler, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		BindNativeTypes(llvm, compiler.ast);
		GenerateLiterals(llvm, compiler.ast);

		for (AST::Id moduleId : ecs::ListAll<AST::CModule>(compiler.ast))
		{
			GenerateIRModule(compiler, moduleId, llvm, builder);
		}
	}
}    // namespace rift::compiler::LLVM
