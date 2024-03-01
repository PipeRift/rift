// Copyright 2015-2023 Piperift - All rights reserved

#include "IRGeneration.h"

#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>
#include <Pipe/Core/String.h>
#include <Pipe/Core/StringView.h>


namespace rift::MIR
{
	const p::TSet<p::Tag> CGenerator::reservedNames{"class", "struct"};


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
		const p::Tag name     = ast::GetModuleName(compiler.ast, moduleId);
		CMIRModule& mirModule = compiler.ast.Add<CMIRModule>(moduleId);
		code                  = &mirModule.code;


		// Get all rift types from the module
		p::TArray<ast::Id> typeIds;
		p::GetIdChildren(access, moduleId, typeIds);
		ExcludeIdsWithout<ast::CDeclType>(access, typeIds);

		{    // Native declarations
			p::TArray<ast::Id> cStructIds = p::FindIdsWith<CDeclCStruct>(access, typeIds);
			p::TArray<ast::Id> cStaticIds = p::FindIdsWith<CDeclCStatic>(access, typeIds);
			p::TArray<ast::Id> cFunctionIds;
			p::GetIdChildren(access, cStaticIds, cFunctionIds);
			ExcludeIdsWithout<ast::CDeclFunction>(access, cFunctionIds);
			DeclareStructs(cStructIds);
			DeclareFunctions(cFunctionIds, false);
		}

		p::TArray<ast::Id> staticFunctionIds;
		{    // Rift declarations & definitions
			p::TArray<ast::Id> structIds = p::FindIdsWith<ast::CDeclStruct>(access, typeIds);
			p::TArray<ast::Id> staticIds = p::FindIdsWith<ast::CDeclStatic>(access, typeIds);
			p::TArray<ast::Id> classIds  = p::FindIdsWith<ast::CDeclClass>(access, typeIds);
			p::TArray<ast::Id> classFunctionIds;
			p::GetIdChildren(access, staticIds, staticFunctionIds);
			p::GetIdChildren(access, classIds, classFunctionIds);
			ExcludeIdsWithout<ast::CDeclFunction>(access, staticFunctionIds);
			ExcludeIdsWithout<ast::CDeclFunction>(access, classFunctionIds);
			p::TArray<ast::Id> functionIds;
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
		p::String strValue;
		for (ast::Id id : FindAllIdsWith<ast::CLiteralIntegral>(access))
		{
			strValue.clear();
			const auto& integral = access.Get<const ast::CLiteralIntegral>(id);
			p::Strings::ToString(strValue, integral.value);
			access.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
		for (ast::Id id : FindAllIdsWith<ast::CLiteralFloating>(access))
		{
			strValue.clear();
			const auto& floating = access.Get<const ast::CLiteralFloating>(id);
			p::Strings::ToString(strValue, floating.value);
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

	void CGenerator::DeclareStructs(p::TView<ast::Id> ids)
	{
		code->append("// Struct Declarations\n");
		for (ast::Id id : ids)
		{
			p::Tag name = ast::GetNameUnsafe(access, id);
			access.Add(id, CMIRType{name});
			p::Strings::FormatTo(*code, "typedef struct {0} {0};\n", name);
		}
		code->push_back('\n');
	}

	void CGenerator::DefineStructs(p::TView<ast::Id> ids)
	{
		code->append("// Struct Definitions\n");
		p::String membersCode;
		p::TArray<ast::Id> memberIds;
		for (ast::Id id : ids)
		{
			membersCode.clear();
			memberIds.Clear(false);
			p::GetIdChildren(access, id, memberIds);

			p::ExcludeIdsWithout<ast::CDeclVariable>(access, memberIds);
			for (ast::Id memberId : memberIds)
			{
				const auto& var = access.Get<const ast::CDeclVariable>(memberId);

				const p::Tag memberName = ast::GetName(access, memberId);
				auto* irType            = access.TryGet<const CMIRType>(var.typeId);
				if (!irType) [[unlikely]]
				{
					const p::Tag typeName = ast::GetName(access, id);
					compiler.Error(p::Strings::Format(
					    "Variable '{}' in struct '{}' has an invalid type", memberName, typeName));
				}
				else if (reservedNames.Contains(memberName)) [[unlikely]]
				{
					const p::Tag typeName = ast::GetName(access, id);
					compiler.Error(p::Strings::Format(
					    "Variable name '{}' not allowed in struct '{}' ", memberName, typeName));
				}
				else
				{
					p::Strings::FormatTo(membersCode, "{} {};\n", irType->value, memberName);
				}
			}

			const auto& type = access.Get<const CMIRType>(id);
			p::Strings::FormatTo(*code, "struct {0} {{\n{1}}};\n", type.value, membersCode);
		}
		code->push_back('\n');
	}

	void CGenerator::DeclareFunctions(p::TView<ast::Id> ids, bool useFullName)
	{
		code->append("// Function Declarations\n");

		for (ast::Id id : ids)
		{
			auto& signature = access.Add<CMIRFunctionSignature>(id).value;

			signature.append("void ");
			const p::String name = useFullName ? ast::GetFullName(access, id, false, '_')
			                                   : p::String{ast::GetName(access, id).AsString()};
			signature.append(name);
			signature.push_back('(');

			if (auto* outputs = access.TryGet<const ast::CExprOutputs>(id))
			{
				for (p::i32 i = 0; i < outputs->pinIds.Size(); ++i)
				{
					ast::Id inputId = outputs->pinIds[i];
					if (access.Has<ast::CInvalid>(inputId))
					{
						continue;
					}

					p::Tag inputName = ast::GetName(access, inputId);

					auto* exprId = access.TryGet<const ast::CExprTypeId>(inputId);
					const auto* irType =
					    exprId ? access.TryGet<const CMIRType>(exprId->id) : nullptr;
					if (!irType) [[unlikely]]
					{
						const p::String functionName = ast::GetFullName(access, id);
						compiler.Error(p::Strings::Format(
						    "Input '{}' in function '{}' has an invalid type. Using i32 instead.",
						    inputName, functionName));
					}
					else if (reservedNames.Contains(inputName)) [[unlikely]]
					{
						const p::String functionName = ast::GetFullName(access, id);
						compiler.Error(
						    p::Strings::Format("Input name '{}' not allowed in function '{}' ",
						        inputName, functionName));
					}
					else
					{
						p::Strings::FormatTo(signature, "{0} {1}, ", irType->value, inputName);
					}
				}
				p::Strings::RemoveFromEnd(signature, ", ");
			}
			signature.push_back(')');

			// Create function
			code->append(signature);
			code->append(";\n");
		}
		code->push_back('\n');
	}

	void CGenerator::DefineFunctions(p::TView<ast::Id> ids)
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
		p::TArray<ast::Id> stmtIds;
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
			compiler.Error(p::Strings::Format(
			    "Call to an invalid function: '{}'", ast::GetFullName(access, functionId)));
			return;
		}

		if (auto* inputs = access.TryGet<const ast::CExprInputs>(id))
		{
			for (p::i32 i = 0; i < inputs->linkedOutputs.Size(); ++i)
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
			p::Strings::RemoveFromEnd(*code, ", ");
		}
		code->push_back(')');
	}

	void CGenerator::CreateMain(ast::Id functionId)
	{
		if (p::IsNone(functionId))
		{
			compiler.Error(p::Strings::Format("Module is executable but has no \"Main\" function"));
			return;
		}

		// auto* customMainFunction = access.Get<const CIRFunction>(functionId).instance;

		code->append("int main() {\nProject_Main_Main();\nreturn 0;\n}\n");
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
