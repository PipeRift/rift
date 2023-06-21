// Copyright 2015-2023 Piperift - All rights reserved

#include "LLVMBackend/IRGeneration.h"

#include "Components/CDeclCStruct.h"
#include "LLVMBackend/Components/CIRFunction.h"
#include "LLVMBackend/LLVMHelpers.h"

#include <AST/Id.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>
#include <Components/CDeclCStatic.h>
#include <Components/CDeclCStruct.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <Pipe/ECS/Utils/Hierarchy.h>


namespace rift::LLVM
{
	void GenerateIR(Compiler& compiler, llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		IRAccess access{compiler.ast};
		BindNativeTypes(llvm, access);
		GenerateLiterals(llvm, access);

		for (AST::Id moduleId : FindAllIdsWith<AST::CModule>(access))
		{
			GenerateIRModule(compiler, access, moduleId, llvm, builder);
		}
	}

	void GenerateIRModule(Compiler& compiler, IRAccess access, AST::Id moduleId,
	    llvm::LLVMContext& llvm, llvm::IRBuilder<>& builder)
	{
		ZoneScoped;
		auto& ast = compiler.ast;

		const Tag name = AST::GetModuleName(compiler.ast, moduleId);

		const auto& module  = compiler.ast.Get<const AST::CModule>(moduleId);
		CIRModule& irModule = compiler.ast.Add<CIRModule>(moduleId);
		irModule.instance   = MakeOwned<llvm::Module>(ToLLVM(name), llvm);

		ModuleIRGen gen{compiler, *irModule.instance.Get(), llvm, builder};

		AST::Id mainFunctionId = AST::NoId;

		// Get all rift types from the module
		TArray<AST::Id> typeIds;
		ecs::GetChildren(ast, moduleId, typeIds);
		ExcludeIdsWithout<AST::CDeclType>(ast, typeIds);

		{    // Native declarations
			TArray<AST::Id> cStructIds = FindIdsWith<CDeclCStruct>(ast, typeIds);
			TArray<AST::Id> cStaticIds = FindIdsWith<CDeclCStatic>(ast, typeIds);
			TArray<AST::Id> cFunctionIds;
			p::ecs::GetChildren(ast, cStaticIds, cFunctionIds);
			ExcludeIdsWithout<AST::CDeclFunction>(ast, cFunctionIds);
			DeclareStructs(gen, ast, cStructIds);
			DeclareFunctions(gen, ast, cFunctionIds, false);
		}

		{    // Rift declarations & definitions
			TArray<AST::Id> structIds = FindIdsWith<AST::CDeclStruct>(ast, typeIds);
			TArray<AST::Id> staticIds = FindIdsWith<AST::CDeclStatic>(ast, typeIds);
			TArray<AST::Id> classIds  = FindIdsWith<AST::CDeclClass>(ast, typeIds);
			TArray<AST::Id> staticFunctionIds;
			TArray<AST::Id> classFunctionIds;
			p::ecs::GetChildren(ast, staticIds, staticFunctionIds);
			p::ecs::GetChildren(ast, classIds, classFunctionIds);
			ExcludeIdsWithout<AST::CDeclFunction>(ast, staticFunctionIds);
			ExcludeIdsWithout<AST::CDeclFunction>(ast, classFunctionIds);
			TArray<AST::Id> functionIds;
			functionIds.Append(staticFunctionIds);
			functionIds.Append(classFunctionIds);

			DeclareStructs(gen, ast, structIds);
			DeclareStructs(gen, ast, classIds);
			DeclareFunctions(gen, ast, functionIds);

			DefineStructs(gen, ast, structIds);
			DefineStructs(gen, ast, classIds);
			DefineFunctions(gen, ast, functionIds);

			mainFunctionId = FindMainFunction(access, staticFunctionIds);
		}

		if (module.target == AST::RiftModuleTarget::Executable)
		{
			CreateMain(gen, access, mainFunctionId);
		}
	}


	void BindNativeTypes(llvm::LLVMContext& llvm, IRAccess access)
	{
		const auto& nativeTypes = static_cast<AST::Tree&>(access.GetContext()).GetNativeTypes();
		access.Add(nativeTypes.boolId, CIRType{llvm::Type::getInt8Ty(llvm)});
		access.Add(nativeTypes.floatId, CIRType{llvm::Type::getFloatTy(llvm)});
		access.Add(nativeTypes.doubleId, CIRType{llvm::Type::getDoubleTy(llvm)});
		access.Add(nativeTypes.u8Id, CIRType{llvm::Type::getInt8Ty(llvm)});
		access.Add(nativeTypes.i8Id, CIRType{llvm::Type::getInt8Ty(llvm)});
		access.Add(nativeTypes.u16Id, CIRType{llvm::Type::getInt16Ty(llvm)});
		access.Add(nativeTypes.i16Id, CIRType{llvm::Type::getInt16Ty(llvm)});
		access.Add(nativeTypes.u32Id, CIRType{llvm::Type::getInt32Ty(llvm)});
		access.Add(nativeTypes.i32Id, CIRType{llvm::Type::getInt32Ty(llvm)});
		access.Add(nativeTypes.u64Id, CIRType{llvm::Type::getInt64Ty(llvm)});
		access.Add(nativeTypes.i64Id, CIRType{llvm::Type::getInt64Ty(llvm)});
		// access.Add<CIRType>(nativeTypes.stringId, {});
	}

	void GenerateLiterals(llvm::LLVMContext& llvm, IRAccess access)
	{
		for (AST::Id id : FindAllIdsWith<AST::CLiteralBool>(access))
		{
			const auto& boolean = access.Get<const AST::CLiteralBool>(id);
			llvm::Value* value  = llvm::ConstantInt::get(llvm, llvm::APInt(1, boolean.value, true));
			access.Add(id, CIRValue{value});
		}
		for (AST::Id id : FindAllIdsWith<AST::CLiteralIntegral>(access))
		{
			const auto& integral = access.Get<const AST::CLiteralIntegral>(id);
			llvm::Value* value   = llvm::ConstantInt::get(
                llvm, llvm::APInt(integral.GetSize(), integral.value, integral.IsSigned()));
			access.Add(id, CIRValue{value});
		}
		for (AST::Id id : FindAllIdsWith<AST::CLiteralFloating>(access))
		{
			const auto& floating = access.Get<const AST::CLiteralFloating>(id);
			llvm::Value* value =
			    llvm::ConstantFP::get(llvm, llvm::APFloat(floating.type == AST::FloatingType::F32
			                                                  ? static_cast<float>(floating.value)
			                                                  : floating.value));
			access.Add(id, CIRValue{value});
		}
		for (AST::Id id : FindAllIdsWith<AST::CLiteralString>(access))
		{
			const auto& string = access.Get<const AST::CLiteralString>(id);
			access.Add(
			    id, CIRValue{llvm::ConstantDataArray::getString(llvm, ToLLVM(string.value))});
		}
	}

	void DeclareStructs(ModuleIRGen& gen, IRAccess access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		for (AST::Id id : ids)
		{
			p::String name = AST::GetFullName(access, id);
			access.Add(id, CIRType{llvm::StructType::create(gen.llvm, ToLLVM(name))});
		}
	}

	void DefineStructs(ModuleIRGen& gen, IRAccess access, TSpan<AST::Id> ids)
	{
		ZoneScoped;
		TArray<AST::Id> memberIds;
		TArray<llvm::Type*> memberTypes;
		for (AST::Id id : ids)
		{
			auto* irStruct = static_cast<llvm::StructType*>(access.Get<const CIRType>(id));

			// Add members
			memberIds.Clear(false);
			memberTypes.Clear(false);
			p::ecs::GetChildren(access, id, memberIds);
			ExcludeIdsWithout<AST::CDeclVariable>(access, memberIds);
			for (AST::Id memberId : memberIds)
			{
				const auto& var = access.Get<const AST::CDeclVariable>(memberId);
				if (auto* irType = access.TryGet<const CIRType>(var.typeId))
				{
					memberTypes.Add(*irType);
				}
				else
				{
					const Tag memberName = AST::GetName(access, memberId);
					const Tag typeName   = AST::GetName(access, id);
					gen.compiler.AddError(Strings::Format(
					    "Variable '{}' in struct '{}' has an invalid type", memberName, typeName));
				}
			}
			irStruct->setBody(ToLLVM(memberTypes));
		}
	}

	void DeclareFunctions(ModuleIRGen& gen, IRAccess access, TSpan<AST::Id> ids, bool useFullName)
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
					if (irType && *irType)
					{
						inputTypes.Add(*irType);
					}
					else
					{
						const Tag argName         = AST::GetName(access, inputId);
						const String functionName = AST::GetFullName(access, id);
						gen.compiler.AddError(Strings::Format(
						    "Input '{}' in function '{}' has an invalid type. Using i32 instead.",
						    argName, functionName));
						inputTypes.Add(gen.builder.getInt32Ty());
					}
				}
			}

			// Create function
			p::String name = useFullName ? AST::GetFullName(access, id)
			                             : p::String{AST::GetName(access, id).AsString()};
			auto* functionType =
			    llvm::FunctionType::get(gen.builder.getVoidTy(), ToLLVM(inputTypes), false);
			functionComp.instance = llvm::Function::Create(
			    functionType, llvm::Function::ExternalLinkage, ToLLVM(name), &gen.module);

			// Set argument names
			i32 i            = 0;
			const auto& args = functionComp.instance->args();
			for (auto& arg : args)
			{
				Tag name = AST::GetName(access, inputIds[i++]);
				arg.setName(ToLLVM(name));
			}

			// Cache final inputs
			functionComp.inputs   = {args.begin(), args.end()};
			functionComp.inputIds = inputIds;
			inputIds.Clear(false);
			inputTypes.Clear(false);
		}
	}

	void DefineFunctions(ModuleIRGen& gen, IRAccess access, TSpan<AST::Id> ids)
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

	void AddStmtBlock(ModuleIRGen& gen, IRAccess access, AST::Id firstStmtId,
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

	llvm::Value* AddExpr(ModuleIRGen& gen, IRAccess access, const AST::ExprOutput& output)
	{
		const auto* value =
		    !IsNone(output.pinId) ? access.TryGet<const CIRValue>(output.pinId) : nullptr;
		if (value)
		{
			return *value;
		}
		return nullptr;
	}

	llvm::BasicBlock* AddIf(
	    ModuleIRGen& gen, IRAccess access, AST::Id id, const CIRFunction& function)
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

	void AddCall(ModuleIRGen& gen, AST::Id id, const AST::CExprCallId& call, IRAccess access)
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


	AST::Id FindMainFunction(IRAccess access, p::TSpan<AST::Id> functionIds)
	{
		static const p::Tag mainFunctionName{"Main"};

		for (AST::Id id : functionIds)
		{
			const auto* ns = access.TryGet<const AST::CNamespace>(id);
			if (ns && ns->name == mainFunctionName)
			{
				return id;
			}
		}
		return AST::NoId;
	}

	void CreateMain(ModuleIRGen& gen, IRAccess access, AST::Id functionId)
	{
		if (p::IsNone(functionId))
		{
			gen.compiler.AddError(
			    Strings::Format("Module is executable but has no \"Main\" function"));
			return;
		}

		auto* customMainFunction = access.Get<const CIRFunction>(functionId).instance;

		auto* mainType = llvm::FunctionType::get(gen.builder.getInt32Ty(), false);
		auto* function =
		    llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "Main", &gen.module);


		auto* entry = llvm::BasicBlock::Create(gen.llvm, "entry", function);
		gen.builder.SetInsertPoint(entry);
		gen.builder.CreateCall(customMainFunction);

		gen.builder.CreateRet(llvm::ConstantInt::get(gen.llvm, llvm::APInt(32, 0)));
	}
}    // namespace rift::LLVM
