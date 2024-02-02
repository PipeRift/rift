// Copyright 2015-2023 Piperift - All rights reserved

#include "IRGeneration.h"

#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"

#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>


namespace rift::MIR
{
	void GenerateC(Compiler& compiler)
	{
		MIRAccess access{compiler.ast};
		CGenerator cGen{compiler, access};
		cGen.BindNativeTypes();
		cGen.GenerateLiterals();
		for (ast::Id moduleId : FindAllIdsWith<ast::CModule>(access))
		{
			cGen.GenerateModule(moduleId);
		}
	}

	void CGenerator::GenerateModule(ast::Id moduleId)
	{
		const Tag name        = ast::GetModuleName(compiler.ast, moduleId);
		CMIRModule& mirModule = compiler.ast.Add<CMIRModule>(moduleId);
		code                  = &mirModule.code;


		// Get all rift types from the module
		TArray<ast::Id> typeIds;
		p::GetIdChildren(access, moduleId, typeIds);
		ExcludeIdsWithout<ast::CDeclType>(access, typeIds);

		{    // Native declarations
			TArray<ast::Id> cStructIds = FindIdsWith<CDeclCStruct>(access, typeIds);
			TArray<ast::Id> cStaticIds = FindIdsWith<CDeclCStatic>(access, typeIds);
			TArray<ast::Id> cFunctionIds;
			p::GetIdChildren(access, cStaticIds, cFunctionIds);
			ExcludeIdsWithout<ast::CDeclFunction>(access, cFunctionIds);
			DeclareStructs(cStructIds);
			DeclareFunctions(cFunctionIds, false);
		}

		TArray<ast::Id> staticFunctionIds;
		{    // Rift declarations & definitions
			TArray<ast::Id> structIds = FindIdsWith<ast::CDeclStruct>(access, typeIds);
			TArray<ast::Id> staticIds = FindIdsWith<ast::CDeclStatic>(access, typeIds);
			TArray<ast::Id> classIds  = FindIdsWith<ast::CDeclClass>(access, typeIds);
			TArray<ast::Id> classFunctionIds;
			p::GetIdChildren(access, staticIds, staticFunctionIds);
			p::GetIdChildren(access, classIds, classFunctionIds);
			ExcludeIdsWithout<ast::CDeclFunction>(access, staticFunctionIds);
			ExcludeIdsWithout<ast::CDeclFunction>(access, classFunctionIds);
			TArray<ast::Id> functionIds;
			functionIds.Append(staticFunctionIds);
			functionIds.Append(classFunctionIds);

			DeclareStructs(structIds);
			DeclareStructs(classIds);
			DeclareFunctions(functionIds);

			DefineStructs(structIds);
			DefineStructs(classIds);
			DefineFunctions(functionIds);
		}

		ast::Id mainFunctionId = ast::NoId;
		const auto& module     = compiler.ast.Get<const ast::CModule>(moduleId);
		if (module.target == ast::RiftModuleTarget::Executable)
		{
			mainFunctionId = FindMainFunction(staticFunctionIds);
			CreateMain(mainFunctionId);
		}
		p::Info(*code);
	}

	void CGenerator::BindNativeTypes()
	{
		const auto& nativeTypes = static_cast<ast::Tree&>(access.GetContext()).GetNativeTypes();
		access.Add(nativeTypes.boolId, CMIRType{"char"});
		access.Add(nativeTypes.floatId, CMIRType{"float"});
		access.Add(nativeTypes.doubleId, CMIRType{"double"});
		access.Add(nativeTypes.u8Id, CMIRType{"unsigned char"});
		access.Add(nativeTypes.i8Id, CMIRType{"char"});
		access.Add(nativeTypes.u16Id, CMIRType{"unsigned short"});
		access.Add(nativeTypes.i16Id, CMIRType{"short"});
		access.Add(nativeTypes.u32Id, CMIRType{"unsigned long"});
		access.Add(nativeTypes.i32Id, CMIRType{"long"});
		access.Add(nativeTypes.u64Id, CMIRType{"unsigned long long"});
		access.Add(nativeTypes.i64Id, CMIRType{"long long"});
		// access.Add<CIRType>(nativeTypes.stringId, {});
	}

	void CGenerator::GenerateLiterals()
	{
		for (ast::Id id : FindAllIdsWith<ast::CLiteralBool>(access))
		{
			const auto& boolean = access.Get<const ast::CLiteralBool>(id);
			access.Add(id, CMIRLiteral{.value = boolean.value ? "true" : "false"});
		}
		String strValue;
		for (ast::Id id : FindAllIdsWith<ast::CLiteralIntegral>(access))
		{
			strValue.clear();
			const auto& integral = access.Get<const ast::CLiteralIntegral>(id);
			Strings::ToString(strValue, integral.value);
			access.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
		for (ast::Id id : FindAllIdsWith<ast::CLiteralFloating>(access))
		{
			strValue.clear();
			const auto& floating = access.Get<const ast::CLiteralFloating>(id);
			Strings::ToString(strValue, floating.value);
			if (floating.type == ast::FloatingType::F32)
			{
				strValue.push_back('f');
			}
			access.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
		for (ast::Id id : FindAllIdsWith<ast::CLiteralString>(access))
		{
			strValue.clear();
			const auto& string = access.Get<const ast::CLiteralString>(id);
			strValue.push_back('\"');
			strValue.append(string.value);
			strValue.push_back('\"');
			access.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
	}

	void CGenerator::DeclareStructs(TView<ast::Id> ids)
	{
		code->append("// Struct Declarations\n");
		for (ast::Id id : ids)
		{
			p::Tag name = ast::GetNameUnsafe(access, id);
			access.Add(id, CMIRType{name});
			Strings::FormatTo(*code, "typedef struct {0} {0};\n", name);
		}
		code->push_back('\n');
	}

	void CGenerator::DefineStructs(TView<ast::Id> ids)
	{
		code->append("// Struct Definitions\n");
		p::String membersCode;
		TArray<ast::Id> memberIds;
		for (ast::Id id : ids)
		{
			membersCode.clear();
			memberIds.Clear(false);
			p::GetIdChildren(access, id, memberIds);

			ExcludeIdsWithout<ast::CDeclVariable>(access, memberIds);
			for (ast::Id memberId : memberIds)
			{
				const auto& var = access.Get<const ast::CDeclVariable>(memberId);

				const Tag memberName = ast::GetName(access, memberId);
				if (auto* irType = access.TryGet<const CMIRType>(var.typeId))
				{
					Strings::FormatTo(membersCode, "{} {};\n", irType->value, memberName);
				}
				else
				{
					const Tag typeName = ast::GetName(access, id);
					compiler.Error(Strings::Format(
					    "Variable '{}' in struct '{}' has an invalid type", memberName, typeName));
				}
			}

			const auto& type = access.Get<const CMIRType>(id);
			Strings::FormatTo(*code, "typedef struct {0} {0} {{\n{1}}}\n", type.value, membersCode);
		}
		code->push_back('\n');
	}

	void CGenerator::DeclareFunctions(TView<ast::Id> ids, bool useFullName)
	{
		code->append("// Function Declarations\n");

		for (ast::Id id : ids)
		{
			auto& signature = access.Add<CMIRFunctionSignature>(id).value;

			signature.append("void ");
			const p::String name = useFullName ? ast::GetFullName(access, id)
			                                   : p::String{ast::GetName(access, id).AsString()};
			signature.append(name);
			signature.push_back('(');

			if (auto* outputs = access.TryGet<const ast::CExprOutputs>(id))
			{
				for (i32 i = 0; i < outputs->pinIds.Size(); ++i)
				{
					ast::Id inputId = outputs->pinIds[i];
					if (access.Has<ast::CInvalid>(inputId))
					{
						continue;
					}

					Tag inputName = ast::GetName(access, inputId);

					auto* exprId = access.TryGet<const ast::CExprTypeId>(inputId);
					const auto* irType =
					    exprId ? access.TryGet<const CMIRType>(exprId->id) : nullptr;
					if (irType) [[likely]]
					{
						Strings::FormatTo(signature, "{0} {1}, ", inputName, irType->value);
					}
					else
					{
						const String functionName = ast::GetFullName(access, id);
						compiler.Error(Strings::Format(
						    "Input '{}' in function '{}' has an invalid type. Using i32 instead.",
						    inputName, functionName));
					}
				}
				Strings::RemoveCharFromEnd(signature, ',');
			}
			signature.push_back(')');

			// Create function
			code->append(signature);
			code->append(";\n");
		}
		code->push_back('\n');
	}

	void CGenerator::DefineFunctions(TView<ast::Id> ids)
	{
		code->append("// Function Definitions\n");
		for (ast::Id id : ids)
		{
			const p::String& signature = access.Get<const CMIRFunctionSignature>(id).value;
			code->append(signature);
			code->append(" {\n");

			const auto& output = access.Get<const ast::CStmtOutput>(id);
			AddStmtBlock(output.linkInputNode);

			code->append("}\n");
		}
		code->push_back('\n');
	}

	void CGenerator::AddStmtBlock(ast::Id firstStmtId)
	{
		ast::Id splitId = ast::NoId;
		TArray<ast::Id> stmtIds;
		ast::GetStmtChain(access, firstStmtId, stmtIds, splitId);

		for (ast::Id id : stmtIds)
		{
			if (const auto* call = access.TryGet<const ast::CExprCallId>(id))
			{
				AddCall(id, *call);
			}
		}

		if (splitId != ast::NoId)
		{
			if (access.Has<const ast::CStmtIf>(splitId))
			{
				AddStmtIf(splitId);
			}
		}
		// TODO: Resolve continuation block and generate it
	}

	void CGenerator::AddExpr(const ast::ExprOutput& output)
	{
		const auto* value =
		    !IsNone(output.pinId) ? access.TryGet<const CMIRLiteral>(output.pinId) : nullptr;
		// TODO
	}

	void CGenerator::AddStmtIf(ast::Id id)
	{
		const auto& outputs      = access.Get<const ast::CStmtOutputs>(id);
		const auto& connectedIds = outputs.linkInputNodes;
		Check(connectedIds.Size() == 2);
		const auto& exprInputs = access.Get<const ast::CExprInputs>(id);
		Check(exprInputs.linkedOutputs.Size() == 1);

		code->append("if (");
		AddExpr(exprInputs.linkedOutputs.First());
		code->append("){\n");
		AddStmtBlock(connectedIds[0]);
		code->append("} else {\n");
		AddStmtBlock(connectedIds[1]);
		code->append("}\n");
	}

	void CGenerator::AddCall(ast::Id id, const ast::CExprCallId& call)
	{
		const ast::Id functionId = call.functionId;
		if (!access.IsValid(functionId))
		{
			compiler.Error("Call to an unknown function");
			return;
		}
		if (!Ensure(access.Has<const CMIRFunctionSignature>(functionId)))
		{
			compiler.Error(Strings::Format(
			    "Call to an invalid function: '{}'", ast::GetName(access, functionId)));
			return;
		}

		if (auto* inputs = access.TryGet<const ast::CExprInputs>(id))
		{
			for (i32 i = 0; i < inputs->linkedOutputs.Size(); ++i)
			{
				ast::ExprOutput output = inputs->linkedOutputs[i];
				if (!output.IsNone())
				{
					AddExpr(output);
					code->push_back(',');
				}
				else
				{
					// TODO: Error? or assign default value?
				}
			}
			Strings::RemoveFromEnd(*code, ", ");
		}
		code->push_back(')');
	}

	void CGenerator::CreateMain(ast::Id functionId)
	{
		if (p::IsNone(functionId))
		{
			compiler.Error(Strings::Format("Module is executable but has no \"Main\" function"));
			return;
		}

		// auto* customMainFunction = access.Get<const CIRFunction>(functionId).instance;

		code->append("void main() {\nMain();\nreturn 0;\n}\n");
	}

	ast::Id CGenerator::FindMainFunction(p::TView<ast::Id> functionIds)
	{
		static const p::Tag mainFunctionName{"Main"};

		for (ast::Id id : functionIds)
		{
			const auto* ns = access.TryGet<const ast::CNamespace>(id);
			if (ns && ns->name == mainFunctionName)
			{
				return id;
			}
		}
		return ast::NoId;
	}
}    // namespace rift::MIR
