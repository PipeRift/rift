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
	void Generate(Compiler& compiler)
	{
		MIRAccess access{compiler.ast};
		CGenerator cGen{compiler, access};
		cGen.BindNativeTypes();
		cGen.GenerateLiterals();
		for (AST::Id moduleId : FindAllIdsWith<AST::CModule>(access))
		{
			cGen.GenerateModule(moduleId);
		}
	}

	void CGenerator::GenerateModule(AST::Id moduleId)
	{
		const Tag name        = AST::GetModuleName(compiler.ast, moduleId);
		CMIRModule& mirModule = compiler.ast.Add<CMIRModule>(moduleId);
		code                  = &mirModule.code;


		// Get all rift types from the module
		TArray<AST::Id> typeIds;
		p::GetIdChildren(access, moduleId, typeIds);
		ExcludeIdsWithout<AST::CDeclType>(access, typeIds);

		{    // Native declarations
			TArray<AST::Id> cStructIds = FindIdsWith<CDeclCStruct>(access, typeIds);
			TArray<AST::Id> cStaticIds = FindIdsWith<CDeclCStatic>(access, typeIds);
			TArray<AST::Id> cFunctionIds;
			p::GetIdChildren(access, cStaticIds, cFunctionIds);
			ExcludeIdsWithout<AST::CDeclFunction>(access, cFunctionIds);
			DeclareStructs(cStructIds);
			DeclareFunctions(cFunctionIds, false);
		}

		TArray<AST::Id> staticFunctionIds;
		{    // Rift declarations & definitions
			TArray<AST::Id> structIds = FindIdsWith<AST::CDeclStruct>(access, typeIds);
			TArray<AST::Id> staticIds = FindIdsWith<AST::CDeclStatic>(access, typeIds);
			TArray<AST::Id> classIds  = FindIdsWith<AST::CDeclClass>(access, typeIds);
			TArray<AST::Id> classFunctionIds;
			p::GetIdChildren(access, staticIds, staticFunctionIds);
			p::GetIdChildren(access, classIds, classFunctionIds);
			ExcludeIdsWithout<AST::CDeclFunction>(access, staticFunctionIds);
			ExcludeIdsWithout<AST::CDeclFunction>(access, classFunctionIds);
			TArray<AST::Id> functionIds;
			functionIds.Append(staticFunctionIds);
			functionIds.Append(classFunctionIds);

			DeclareStructs(structIds);
			DeclareStructs(classIds);
			DeclareFunctions(functionIds);

			DefineStructs(structIds);
			DefineStructs(classIds);
			DefineFunctions(functionIds);
		}

		AST::Id mainFunctionId = AST::NoId;
		const auto& module     = compiler.ast.Get<const AST::CModule>(moduleId);
		if (module.target == AST::RiftModuleTarget::Executable)
		{
			mainFunctionId = FindMainFunction(staticFunctionIds);
			CreateMain(mainFunctionId);
		}
	}

	void CGenerator::BindNativeTypes()
	{
		const auto& nativeTypes = static_cast<AST::Tree&>(access.GetContext()).GetNativeTypes();
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
		for (AST::Id id : FindAllIdsWith<AST::CLiteralBool>(access))
		{
			const auto& boolean = access.Get<const AST::CLiteralBool>(id);
			access.Add(id, CMIRLiteral{.value = boolean.value ? "true" : "false"});
		}
		String strValue;
		for (AST::Id id : FindAllIdsWith<AST::CLiteralIntegral>(access))
		{
			strValue.clear();
			const auto& integral = access.Get<const AST::CLiteralIntegral>(id);
			Strings::ToString(strValue, integral.value);
			access.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
		for (AST::Id id : FindAllIdsWith<AST::CLiteralFloating>(access))
		{
			strValue.clear();
			const auto& floating = access.Get<const AST::CLiteralFloating>(id);
			Strings::ToString(strValue, floating.value);
			if (floating.type == AST::FloatingType::F32)
			{
				strValue.push_back('f');
			}
			access.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
		for (AST::Id id : FindAllIdsWith<AST::CLiteralString>(access))
		{
			strValue.clear();
			const auto& string = access.Get<const AST::CLiteralString>(id);
			strValue.push_back('\"');
			strValue.append(string.value);
			strValue.push_back('\"');
			access.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
	}

	void CGenerator::DeclareStructs(TView<AST::Id> ids)
	{
		code->append("// Struct Declarations\n");
		for (AST::Id id : ids)
		{
			p::Tag name = AST::GetNameUnsafe(access, id);
			access.Add(id, CMIRType{name});
			Strings::FormatTo(*code, "typedef struct {0} {0};\n", name);
		}
		code->push_back('\n');
	}

	void CGenerator::DefineStructs(TView<AST::Id> ids)
	{
		code->append("// Struct Definitions\n");
		p::String membersCode;
		TArray<AST::Id> memberIds;
		for (AST::Id id : ids)
		{
			membersCode.clear();
			memberIds.Clear(false);
			p::GetIdChildren(access, id, memberIds);

			ExcludeIdsWithout<AST::CDeclVariable>(access, memberIds);
			for (AST::Id memberId : memberIds)
			{
				const auto& var = access.Get<const AST::CDeclVariable>(memberId);

				const Tag memberName = AST::GetName(access, memberId);
				if (auto* irType = access.TryGet<const CMIRType>(var.typeId))
				{
					Strings::FormatTo(membersCode, "{} {};\n", irType->value, memberName);
				}
				else
				{
					const Tag typeName = AST::GetName(access, id);
					compiler.Error(Strings::Format(
					    "Variable '{}' in struct '{}' has an invalid type", memberName, typeName));
				}
			}

			const auto& type = access.Get<const CMIRType>(id);
			Strings::FormatTo(*code, "typedef struct {0} {0} {{\n{1}}}\n", type.value, membersCode);
		}
		code->push_back('\n');
	}

	void CGenerator::DeclareFunctions(TView<AST::Id> ids, bool useFullName)
	{
		code->append("// Function Declarations\n");

		for (AST::Id id : ids)
		{
			auto& signature = access.Add<CMIRFunctionSignature>(id).value;

			signature.append("void ");
			const p::String name = useFullName ? AST::GetFullName(access, id)
			                                   : p::String{AST::GetName(access, id).AsString()};
			signature.append(name);
			signature.push_back('(');

			if (auto* outputs = access.TryGet<const AST::CExprOutputs>(id))
			{
				for (i32 i = 0; i < outputs->pinIds.Size(); ++i)
				{
					AST::Id inputId = outputs->pinIds[i];
					if (access.Has<AST::CInvalid>(inputId))
					{
						continue;
					}

					Tag inputName = AST::GetName(access, inputId);

					AST::Id typeId     = access.Get<const AST::CExprTypeId>(inputId).id;
					const auto* irType = access.TryGet<const CMIRType>(typeId);
					if (irType)
					{
						Strings::FormatTo(signature, "{0} {1}, ", inputName, irType->value);
					}
					else
					{
						const String functionName = AST::GetFullName(access, id);
						compiler.Error(Strings::Format(
						    "Input '{}' in function '{}' has an invalid type. Using i32 instead.",
						    inputName, functionName));
					}
				}
				Strings::RemoveFromEnd(signature, ',');
			}
			signature.push_back(')');

			// Create function
			code->append(signature);
			code->append(";\n");
		}
		code->push_back('\n');
	}

	void CGenerator::DefineFunctions(TView<AST::Id> ids)
	{
		code->append("// Function Definitions\n");
		for (AST::Id id : ids)
		{
			const p::String& signature = access.Get<const CMIRFunctionSignature>(id).value;
			code->append(signature);
			code->append(" {\n");

			const auto& output = access.Get<const AST::CStmtOutput>(id);
			AddStmtBlock(output.linkInputNode);

			code->append("}\n");
		}
		code->push_back('\n');
	}

	void CGenerator::AddStmtBlock(AST::Id firstStmtId)
	{
		AST::Id splitId = AST::NoId;
		TArray<AST::Id> stmtIds;
		AST::GetStmtChain(access, firstStmtId, stmtIds, splitId);

		for (AST::Id id : stmtIds)
		{
			if (const auto* call = access.TryGet<const AST::CExprCallId>(id))
			{
				AddCall(id, *call);
			}
		}

		if (splitId != AST::NoId)
		{
			if (access.Has<const AST::CStmtIf>(splitId))
			{
				AddStmtIf(splitId);
			}
		}
		// TODO: Resolve continuation block and generate it
	}

	void CGenerator::AddExpr(const AST::ExprOutput& output)
	{
		const auto* value =
		    !IsNone(output.pinId) ? access.TryGet<const CMIRLiteral>(output.pinId) : nullptr;
		// TODO
	}

	void CGenerator::AddStmtIf(AST::Id id)
	{
		const auto& outputs      = access.Get<const AST::CStmtOutputs>(id);
		const auto& connectedIds = outputs.linkInputNodes;
		Check(connectedIds.Size() == 2);
		const auto& exprInputs = access.Get<const AST ::CExprInputs>(id);
		Check(exprInputs.linkedOutputs.Size() == 1);

		code->append("if (");
		AddExpr(exprInputs.linkedOutputs.First());
		code->append("){\n");
		AddStmtBlock(connectedIds[0]);
		code->append("} else {\n");
		AddStmtBlock(connectedIds[1]);
		code->append("}\n");
	}

	void CGenerator::AddCall(AST::Id id, const AST::CExprCallId& call)
	{
		const AST::Id functionId = call.functionId;
		if (!access.IsValid(functionId))
		{
			compiler.Error("Call to an unknown function");
			return;
		}
		if (!Ensure(access.Has<const CMIRFunctionSignature>(functionId)))
		{
			compiler.Error(Strings::Format(
			    "Call to an invalid function: '{}'", AST::GetName(access, functionId)));
			return;
		}

		if (auto* inputs = access.TryGet<const AST::CExprInputs>(id))
		{
			for (i32 i = 0; i < inputs->linkedOutputs.Size(); ++i)
			{
				AST::ExprOutput output = inputs->linkedOutputs[i];
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

	void CGenerator::CreateMain(AST::Id functionId)
	{
		if (p::IsNone(functionId))
		{
			compiler.Error(Strings::Format("Module is executable but has no \"Main\" function"));
			return;
		}

		// auto* customMainFunction = access.Get<const CIRFunction>(functionId).instance;

		code->append("void main() {\nMain();\nreturn 0;\n}\n");
	}

	AST::Id CGenerator::FindMainFunction(p::TView<AST::Id> functionIds)
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
}    // namespace rift::MIR
