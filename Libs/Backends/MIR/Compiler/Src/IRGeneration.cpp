// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "IRGeneration.h"

#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>
#include <PipeStrings.h>
#include <Pipe/Core/StringView.h>


namespace rift::MIR
{
	const p::TSet<p::Tag> CGenerator::reservedNames{"class", "struct"};


	void GenerateC(Compiler& compiler)
	{
		MIRScope scope{compiler.ast};
		CGenerator cGen{compiler, scope};
		cGen.BindNativeTypes();
		cGen.GenerateLiterals();
		for (ast::Id moduleId : FindAllIdsWith<ast::CModule>(scope))
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
		p::GetIdChildren(scope, moduleId, typeIds);
		ExcludeIdsWithout<ast::CDeclType>(scope, typeIds);

		{    // Native declarations
			p::TArray<ast::Id> cStructIds = p::FindIdsWith<CDeclCStruct>(scope, typeIds);
			p::TArray<ast::Id> cStaticIds = p::FindIdsWith<CDeclCStatic>(scope, typeIds);
			p::TArray<ast::Id> cFunctionIds;
			p::GetIdChildren(scope, cStaticIds, cFunctionIds);
			ExcludeIdsWithout<ast::CDeclFunction>(scope, cFunctionIds);
			DeclareStructs(cStructIds);
			DeclareFunctions(cFunctionIds, false);
		}

		p::TArray<ast::Id> staticFunctionIds;
		{    // Rift declarations & definitions
			p::TArray<ast::Id> structIds = p::FindIdsWith<ast::CDeclStruct>(scope, typeIds);
			p::TArray<ast::Id> staticIds = p::FindIdsWith<ast::CDeclStatic>(scope, typeIds);
			p::TArray<ast::Id> classIds  = p::FindIdsWith<ast::CDeclClass>(scope, typeIds);
			p::TArray<ast::Id> classFunctionIds;
			p::GetIdChildren(scope, staticIds, staticFunctionIds);
			p::GetIdChildren(scope, classIds, classFunctionIds);
			ExcludeIdsWithout<ast::CDeclFunction>(scope, staticFunctionIds);
			ExcludeIdsWithout<ast::CDeclFunction>(scope, classFunctionIds);
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
		const auto& nativeTypes = static_cast<ast::Tree&>(scope.GetContext()).GetNativeTypes();
		scope.Add(nativeTypes.boolId, CMIRType{"char"});
		scope.Add(nativeTypes.floatId, CMIRType{"float"});
		scope.Add(nativeTypes.doubleId, CMIRType{"double"});
		scope.Add(nativeTypes.u8Id, CMIRType{"unsigned char"});
		scope.Add(nativeTypes.i8Id, CMIRType{"char"});
		scope.Add(nativeTypes.u16Id, CMIRType{"unsigned short"});
		scope.Add(nativeTypes.i16Id, CMIRType{"short"});
		scope.Add(nativeTypes.u32Id, CMIRType{"unsigned long"});
		scope.Add(nativeTypes.i32Id, CMIRType{"long"});
		scope.Add(nativeTypes.u64Id, CMIRType{"unsigned long long"});
		scope.Add(nativeTypes.i64Id, CMIRType{"long long"});
		// scope.Add<CIRType>(nativeTypes.stringId, {});
	}

	void CGenerator::GenerateLiterals()
	{
		for (ast::Id id : FindAllIdsWith<ast::CLiteralBool>(scope))
		{
			const auto& boolean = scope.Get<const ast::CLiteralBool>(id);
			scope.Add(id, CMIRLiteral{.value = boolean.value ? "true" : "false"});
		}
		p::String strValue;
		for (ast::Id id : FindAllIdsWith<ast::CLiteralIntegral>(scope))
		{
			strValue.clear();
			const auto& integral = scope.Get<const ast::CLiteralIntegral>(id);
			p::Strings::ToString(strValue, integral.value);
			scope.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
		for (ast::Id id : FindAllIdsWith<ast::CLiteralFloating>(scope))
		{
			strValue.clear();
			const auto& floating = scope.Get<const ast::CLiteralFloating>(id);
			p::Strings::ToString(strValue, floating.value);
			if (floating.type == ast::FloatingType::F32)
			{
				strValue.push_back('f');
			}
			scope.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
		for (ast::Id id : FindAllIdsWith<ast::CLiteralString>(scope))
		{
			strValue.clear();
			const auto& string = scope.Get<const ast::CLiteralString>(id);
			strValue.push_back('\"');
			strValue.append(string.value);
			strValue.push_back('\"');
			scope.Add(id, CMIRLiteral{.value = p::Tag{strValue}});
		}
	}

	void CGenerator::DeclareStructs(p::TView<ast::Id> ids)
	{
		code->append("// Struct Declarations\n");
		for (ast::Id id : ids)
		{
			p::Tag name = ast::GetNameUnsafe(scope, id);
			scope.Add(id, CMIRType{name});
			p::FormatTo(*code, "typedef struct {0} {0};\n", name);
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
			memberIds.Clear(p::Shrink::No);
			p::GetIdChildren(scope, id, memberIds);

			p::ExcludeIdsWithout<ast::CDeclVariable>(scope, memberIds);
			for (ast::Id memberId : memberIds)
			{
				const auto& var = scope.Get<const ast::CDeclVariable>(memberId);

				const p::Tag memberName = ast::GetName(scope, memberId);
				auto* irType            = scope.TryGet<const CMIRType>(var.typeId);
				if (!irType) [[unlikely]]
				{
					const p::Tag typeName = ast::GetName(scope, id);
					compiler.Error(p::Format(
					    "Variable '{}' in struct '{}' has an invalid type", memberName, typeName));
				}
				else if (reservedNames.Contains(memberName)) [[unlikely]]
				{
					const p::Tag typeName = ast::GetName(scope, id);
					compiler.Error(p::Format(
					    "Variable name '{}' not allowed in struct '{}' ", memberName, typeName));
				}
				else
				{
					p::FormatTo(membersCode, "{} {};\n", irType->value, memberName);
				}
			}

			const auto& type = scope.Get<const CMIRType>(id);
			p::FormatTo(*code, "struct {0} {{\n{1}}};\n", type.value, membersCode);
		}
		code->push_back('\n');
	}

	void CGenerator::DeclareFunctions(p::TView<ast::Id> ids, bool useFullName)
	{
		code->append("// Function Declarations\n");

		for (ast::Id id : ids)
		{
			auto& signature = scope.Add<CMIRFunctionSignature>(id).value;

			signature.append("void ");
			const p::String name = useFullName ? ast::GetFullName(scope, id, false, '_')
			                                   : p::String{ast::GetName(scope, id).AsString()};
			signature.append(name);
			signature.push_back('(');

			if (auto* outputs = scope.TryGet<const ast::CExprOutputs>(id))
			{
				for (p::i32 i = 0; i < outputs->pinIds.Size(); ++i)
				{
					ast::Id inputId = outputs->pinIds[i];
					if (scope.Has<ast::CInvalid>(inputId))
					{
						continue;
					}

					p::Tag inputName = ast::GetName(scope, inputId);

					auto* exprId = scope.TryGet<const ast::CExprTypeId>(inputId);
					const auto* irType =
					    exprId ? scope.TryGet<const CMIRType>(exprId->id) : nullptr;
					if (!irType) [[unlikely]]
					{
						const p::String functionName = ast::GetFullName(scope, id);
						compiler.Error(p::Format(
						    "Input '{}' in function '{}' has an invalid type. Using i32 instead.",
						    inputName, functionName));
					}
					else if (reservedNames.Contains(inputName)) [[unlikely]]
					{
						const p::String functionName = ast::GetFullName(scope, id);
						compiler.Error(
						    p::Format("Input name '{}' not allowed in function '{}' ",
						        inputName, functionName));
					}
					else
					{
						p::FormatTo(signature, "{0} {1}, ", irType->value, inputName);
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
			const p::String& signature = scope.Get<const CMIRFunctionSignature>(id).value;
			code->append(signature);
			code->append(" {\n");

			const auto& output = scope.Get<const ast::CStmtOutput>(id);
			AddStmtBlock(output.linkInputNode);

			code->append("}\n");
		}
		code->push_back('\n');
	}

	void CGenerator::AddStmtBlock(ast::Id firstStmtId)
	{
		ast::Id splitId = ast::NoId;
		p::TArray<ast::Id> stmtIds;
		ast::GetStmtChain(scope, firstStmtId, stmtIds, splitId);

		for (ast::Id id : stmtIds)
		{
			if (const auto* call = scope.TryGet<const ast::CExprCallId>(id))
			{
				AddCall(id, *call);
			}
		}

		if (splitId != ast::NoId)
		{
			if (scope.Has<const ast::CStmtIf>(splitId))
			{
				AddStmtIf(splitId);
			}
		}
		// TODO: Resolve continuation block and generate it
	}

	void CGenerator::AddExpr(const ast::ExprOutput& output)
	{
		const auto* value =
		    !IsNone(output.pinId) ? scope.TryGet<const CMIRLiteral>(output.pinId) : nullptr;
		// TODO
	}

	void CGenerator::AddStmtIf(ast::Id id)
	{
		const auto& outputs      = scope.Get<const ast::CStmtOutputs>(id);
		const auto& connectedIds = outputs.linkInputNodes;
		P_Check(connectedIds.Size() == 2);
		const auto& exprInputs = scope.Get<const ast::CExprInputs>(id);
		P_Check(exprInputs.linkedOutputs.Size() == 1);

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
		if (!scope.IsValid(functionId))
		{
			compiler.Error("Call to an unknown function");
			return;
		}
		if (!P_Ensure(scope.Has<const CMIRFunctionSignature>(functionId)))
		{
			compiler.Error(p::Format(
			    "Call to an invalid function: '{}'", ast::GetFullName(scope, functionId)));
			return;
		}

		if (auto* inputs = scope.TryGet<const ast::CExprInputs>(id))
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
			compiler.Error(p::Format("Module is executable but has no \"Main\" function"));
			return;
		}

		// auto* customMainFunction = scope.Get<const CIRFunction>(functionId).instance;

		code->append("int main() {\nProject_Main_Main();\nreturn 0;\n}\n");
	}

	ast::Id CGenerator::FindMainFunction(p::TView<ast::Id> functionIds)
	{
		static const p::Tag mainFunctionName{"Main"};

		for (ast::Id id : functionIds)
		{
			const auto* ns = scope.TryGet<const ast::CNamespace>(id);
			if (ns && ns->name == mainFunctionName)
			{
				return id;
			}
		}
		return ast::NoId;
	}
}    // namespace rift::MIR
