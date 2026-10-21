// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "IRGeneration.h"

#include <AST/Components/Declarations.h>
#include <AST/Systems/CastSystem.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>
#include <Pipe/Core/StringView.h>
#include <PipeStrings.h>


namespace rift::MIR
{
	const p::TSet<p::Tag> CGenerator::reservedNames{"class", "struct"};

	// Operator strings indexed by their enum value. Binary operators include the
	// surrounding spaces so they are appended in a single operation.
	static constexpr p::StringView unaryOperators[]{"!", "-", "++", "--", "~"};
	static constexpr p::StringView binaryOperators[]{" + ", " - ", " * ", " / ", " % ",
	    " == ", " != ", " > ", " < ", " >= ", " <= ", " && ", " || ", " & ", " | ", " ^ "};

	static_assert(std::size(unaryOperators) == p::sizet(p::u8(ast::UnaryOperatorType::BitNot)) + 1,
	    "unaryOperators must have one entry per UnaryOperatorType");
	static_assert(std::size(binaryOperators) == p::sizet(p::u8(ast::BinaryOperatorType::Xor)) + 1,
	    "binaryOperators must have one entry per BinaryOperatorType");


	static void AppendCStringLiteral(p::String& out, p::StringView value)
	{
		out.push_back('"');
		for (p::sizet i = 0; i < value.size(); ++i)
		{
			switch (value[i])
			{
				case '"': out.append("\\\""); break;
				case '\\': out.append("\\\\"); break;
				case '\n': out.append("\\n"); break;
				case '\r': out.append("\\r"); break;
				case '\t': out.append("\\t"); break;
				default: out.push_back(value[i]); break;
			}
		}
		out.push_back('"');
	}


	void GenerateC(Compiler& compiler)
	{
		MIRScope scope{compiler.ast};
		CGenerator cGen{compiler, scope};
		cGen.AssignGeneratedNames();
		cGen.BindNativeTypes();
		cGen.GenerateLiterals();
		for (ast::Id moduleId : FindAllIdsWith<ast::CModule>(scope))
		{
			cGen.GenerateModule(moduleId);
		}
	}


	namespace
	{
		/** Reserve a name in the scope, adding a numeric suffix when it is already taken. */
		void AssignName(ast::Tree& ast, ast::Id id, p::TSet<p::Tag>& used, bool reserveEnd = false)
		{
			const p::Tag base = ast::GetName(ast, id);
			if (base.IsNone())
			{
				return;
			}

			const p::StringView baseName = base.AsString();
			p::String name;
			p::String endName;
			for (p::i32 suffix = 0;; ++suffix)
			{
				name.clear();
				if (suffix == 0)
				{
					name.append(baseName);
				}
				else
				{
					p::FormatTo(name, "{}_{}", baseName, suffix);
				}

				const p::Tag tag{name};
				if (used.Contains(tag))
				{
					continue;
				}

				if (reserveEnd)
				{
					endName.clear();
					p::FormatTo(endName, "{}_end", name);
					if (used.Contains(p::Tag{endName}))
					{
						continue;
					}
					used.Insert(p::Tag{endName});
				}

				used.Insert(tag);
				ast.GetOrAdd<ast::CGeneratedName>(id).name = tag;
				return;
			}
		}

		void AssignVariables(ast::Tree& ast, ast::Id ownerId, p::TSet<p::Tag>& used)
		{
			p::TArray<ast::Id> children;
			p::GetIdChildren(ast, ownerId, children);
			for (ast::Id childId : children)
			{
				if (ast.Has<ast::CDeclVariable>(childId))
				{
					AssignName(ast, childId, used);
				}
			}
		}

		void AddVariableNames(ast::Tree& ast, ast::Id ownerId, p::TSet<p::Tag>& used)
		{
			p::TArray<ast::Id> children;
			p::GetIdChildren(ast, ownerId, children);
			for (ast::Id childId : children)
			{
				if (ast.Has<ast::CDeclVariable>(childId))
				{
					used.Insert(ast.Get<const ast::CGeneratedName>(childId).name);
				}
			}
		}

		/** Collects loops in statement execution order so the first loop in a function
		 * gets the unsuffixed name. */
		void CollectForIds(p::TIdScopeRef<ast::CStmtFor, ast::CStmtOutput, ast::CStmtOutputs> scope,
		    ast::Id firstStmtId, p::TArray<ast::Id>& forIds, p::TSet<ast::Id>& visited)
		{
			ast::Id current = firstStmtId;
			while (!p::IsNone(current) && !visited.Contains(current))
			{
				visited.Insert(current);

				if (const auto* outputs = scope.TryGet<const ast::CStmtOutputs>(current))
				{
					if (scope.Has<ast::CStmtFor>(current))
					{
						forIds.Add(current);
					}
					for (ast::Id nextId : outputs->linkInputNodes)
					{
						CollectForIds(scope, nextId, forIds, visited);
					}
					return;
				}

				const auto* output = scope.TryGet<const ast::CStmtOutput>(current);
				if (!output)
				{
					return;
				}
				current = output->linkInputNode;
			}
		}

		void AssignFunctionNames(ast::Tree& ast, ast::Id functionId, p::TArray<ast::Id>& forIds,
		    p::TSet<p::Tag>& globalNames)
		{
			// Functions inherit the global scope and their type's member names
			p::TSet<p::Tag> functionNames = globalNames;
			const ast::Id ownerId         = p::GetIdParent(ast, functionId);
			if (!p::IsNone(ownerId))
			{
				AddVariableNames(ast, ownerId, functionNames);
			}

			// Parameters are part of the function scope. Inputs are stored in CExprOutputs
			// and outputs in CExprInputs.
			if (const auto* inputs = ast.TryGet<const ast::CExprOutputs>(functionId))
			{
				for (ast::Id pinId : inputs->pinIds)
				{
					AssignName(ast, pinId, functionNames);
				}
			}
			if (const auto* outputs = ast.TryGet<const ast::CExprInputs>(functionId))
			{
				for (ast::Id pinId : outputs->pinIds)
				{
					AssignName(ast, pinId, functionNames);
				}
			}

			// Locals declared directly in the function share its scope
			AssignVariables(ast, functionId, functionNames);

			// Loop indices and their locals live in the function body
			p::TSet<p::Tag> bodyNames = functionNames;
			for (ast::Id forId : forIds)
			{
				if (const auto* outputs = ast.TryGet<const ast::CExprOutputs>(forId))
				{
					for (p::i32 i = 0; i < outputs->pinIds.Size(); ++i)
					{
						// The first expression output is the loop index. Its end bound is
						// generated as a companion name.
						AssignName(ast, outputs->pinIds[i], bodyNames, i == 0);
					}
				}
				AssignVariables(ast, forId, bodyNames);
			}
		}
	}    // namespace

	void CGenerator::AssignGeneratedNames()
	{
		ast::Tree& ast = compiler.ast;

		// Names are derived from the current graph, never serialized
		for (ast::Id id : p::FindAllIdsWith<ast::CGeneratedName>(ast))
		{
			ast.Remove<ast::CGeneratedName>(id);
		}

		// Globals come first so parameters and locals can avoid them
		p::TSet<p::Tag> globalNames;
		for (ast::Id typeId : p::FindAllIdsWith<ast::CDeclStatic>(ast))
		{
			AssignVariables(ast, typeId, globalNames);
		}

		// Struct and class members have their own scope
		p::TArray<ast::Id> memberTypeIds;
		memberTypeIds.Append(p::FindAllIdsWith<ast::CDeclStruct>(ast));
		memberTypeIds.Append(p::FindAllIdsWith<ast::CDeclClass>(ast));
		for (ast::Id typeId : memberTypeIds)
		{
			p::TSet<p::Tag> memberNames;
			AssignVariables(ast, typeId, memberNames);
		}

		for (ast::Id functionId : p::FindAllIdsWith<ast::CDeclFunction>(ast))
		{
			p::TArray<ast::Id> forIds;
			p::TSet<ast::Id> visited;
			if (const auto* output = ast.TryGet<const ast::CStmtOutput>(functionId))
			{
				CollectForIds(ast, output->linkInputNode, forIds, visited);
			}
			AssignFunctionNames(ast, functionId, forIds, globalNames);
		}
	}

	void CGenerator::GenerateModule(ast::Id moduleId)
	{
		CMIRModule& mirModule = compiler.ast.Add<CMIRModule>(moduleId);
		code                  = &mirModule.code;

		DeclareNativeTypes();


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
		scope.Add(nativeTypes.stringId, CMIRType{"StringView"});
	}

	void CGenerator::DeclareNativeTypes()
	{
		code->append("// Native Types\n");

		// Mirrored by the runtime in Std's Std/IO.h, which must keep the same member order.
		// TODO: Emit p::StringView directly once Pipe defines its own string view, whose
		// layout would be specified instead of depending on the host standard library.
		code->append(
		    "typedef struct StringView"
		    "{"
		    "    const char* data;"
		    "    unsigned long long size;"
		    "} StringView;\n\n");
	}

	void CGenerator::GenerateLiterals()
	{
		for (ast::Id id : FindAllIdsWith<ast::CLiteralBool>(scope))
		{
			const auto& boolean = scope.Get<const ast::CLiteralBool>(id);
			scope.Add(id, CMIRLiteral{.value = boolean.value ? "true" : "false"});
		}
		p::String strValue;
		p::String value;
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
			value.clear();
			AppendCStringLiteral(value, string.value);
			p::FormatTo(strValue, "(StringView){{{0}, {1}}}", value, string.value.size());
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

				const p::Tag memberName = scope.Get<const ast::CGeneratedName>(memberId).name;
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

			// Functions always return void. Inputs are passed by value and outputs are passed by
			// pointer as trailing parameters.
			signature.append("void ");
			const p::String name = useFullName ? ast::GetFullName(scope, id, false, '_')
			                                   : p::String{ast::GetName(scope, id).AsString()};
			signature.append(name);
			signature.push_back('(');

			auto appendParams = [this, &signature, id](
			                        const p::TArray<ast::Id>& pinIds, bool isOutput)
			{
				for (ast::Id pinId : pinIds)
				{
					if (scope.Has<ast::CInvalid>(pinId))
					{
						continue;
					}

					const p::Tag pinName = scope.Get<const ast::CGeneratedName>(pinId).name;
					const auto* exprId   = scope.TryGet<const ast::CExprTypeId>(pinId);
					const auto* irType =
					    exprId ? scope.TryGet<const CMIRType>(exprId->id) : nullptr;
					if (!irType) [[unlikely]]
					{
						const auto* pinKind          = isOutput ? "Output" : "Input";
						const p::String functionName = ast::GetFullName(scope, id);
						compiler.Error(p::Format("{} '{}' in function '{}' has an invalid type",
						    pinKind, pinName, functionName));
					}
					else if (reservedNames.Contains(pinName)) [[unlikely]]
					{
						const auto* pinKind          = isOutput ? "Output" : "Input";
						const p::String functionName = ast::GetFullName(scope, id);
						compiler.Error(p::Format("{} name '{}' not allowed in function '{}' ",
						    pinKind, pinName, functionName));
					}
					else
					{
						p::FormatTo(
						    signature, "{0}{1} {2}, ", irType->value, isOutput ? "*" : "", pinName);
					}
				}
			};

			// A function's parameters are stored in CExprOutputs and its results in CExprInputs,
			// which is the opposite of what the names suggest
			if (const auto* paramPins = scope.TryGet<const ast::CExprOutputs>(id))
			{
				appendParams(paramPins->pinIds, false);
			}
			if (const auto* outputPins = scope.TryGet<const ast::CExprInputs>(id))
			{
				appendParams(outputPins->pinIds, true);
			}
			p::Strings::RemoveFromEnd(signature, ", ");
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
				code->append(";\n");
			}
		}

		if (splitId != ast::NoId)
		{
			if (scope.Has<ast::CStmtIf>(splitId))
			{
				AddStmtIf(splitId);
			}
			else if (scope.Has<ast::CStmtFor>(splitId))
			{
				AddStmtFor(splitId);
			}
		}
		// TODO: Resolve continuation block and generate it
	}

	void CGenerator::AddExpr(const ast::ExprOutput& output, ast::Id pinId)
	{
		if (output.IsNone())
		{
			compiler.Error("Missing expression found while generating code");
			return;
		}

		// The value is cast into the type of the pin it feeds when their types differ
		const ast::Id sourceType = ast::GetPinType(scope, output.pinId);
		const ast::Id targetType = ast::GetPinType(scope, pinId);
		if (!p::IsNone(pinId) && !p::IsNone(sourceType) && !p::IsNone(targetType)
		    && sourceType != targetType)
		{
			if (!ast::CastSystem::CanCastImplicit(scope, sourceType, targetType)) [[unlikely]]
			{
				const p::StringView fromName = ast::GetName(compiler.ast, sourceType).AsString();
				const p::StringView toName   = ast::GetName(compiler.ast, targetType).AsString();
				compiler.Error(
				    ast::CastSystem::CanCast(scope, sourceType, targetType)
				        ? p::Format("'{}' needs an explicit cast into '{}'", fromName, toName)
						: p::Format("'{}' can't cast into '{}'", fromName, toName));
				// Emit the value as it is, so generation can continue
			}
			else if (const auto* irType = scope.TryGet<const CMIRType>(targetType))
			{
				// Every cast is expressed as a C cast, no matter which instruction a
				// backend would lower it to
				code->append("((");
				code->append(irType->value.AsString());
				code->append(")(");
				AddExpr(output);
				code->append("))");
				return;
			}
			else [[unlikely]]
			{
				compiler.Error("Cannot cast a value into an invalid type");
			}
		}

		// Literal nodes are their own output pin
		if (const auto* literal = scope.TryGet<const CMIRLiteral>(output.pinId))
		{
			code->append(literal->value.AsString());
		}
		else if (const auto* call = scope.TryGet<const ast::CExprCallId>(output.nodeId))
		{
			AddCall(output.nodeId, *call);
		}
		else if (const auto* unary = scope.TryGet<const ast::CExprUnaryOperator>(output.nodeId))
		{
			AddUnaryOperator(output.nodeId, *unary);
		}
		else if (const auto* binary = scope.TryGet<const ast::CExprBinaryOperator>(output.nodeId))
		{
			AddBinaryOperator(output.nodeId, *binary);
		}
		else if (scope.Has<ast::CExprCast>(output.nodeId))
		{
			AddCast(output.nodeId);
		}
		else if (scope.Has<ast::CStmtFor>(output.nodeId))
		{
			// The only expression output of a for node is its loop index
			const p::Tag indexName = scope.Get<const ast::CGeneratedName>(output.pinId).name;
			if (indexName.IsNone())
			{
				compiler.Error("For loop index has no generated name");
			}
			else
			{
				code->append(indexName.AsString());
			}
		}
		else
		{
			compiler.Error("Unsupported expression found while generating code");
		}
	}

	void CGenerator::AddUnaryOperator(ast::Id id, const ast::CExprUnaryOperator& op)
	{
		const auto* inputs = scope.TryGet<const ast::CExprInputs>(id);
		if (!inputs || inputs->linkedOutputs.IsEmpty())
		{
			compiler.Error("Unary operator without a connected operand");
			return;
		}

		code->append(unaryOperators[p::u8(op.type)]);
		code->push_back('(');
		AddExpr(inputs->linkedOutputs.First());
		code->push_back(')');
	}

	void CGenerator::AddBinaryOperator(ast::Id id, const ast::CExprBinaryOperator& op)
	{
		const auto* inputs = scope.TryGet<const ast::CExprInputs>(id);
		if (!inputs || inputs->linkedOutputs.Size() < 2)
		{
			compiler.Error("Binary operator without two connected operands");
			return;
		}

		code->push_back('(');
		AddExpr(inputs->linkedOutputs[0]);
		code->append(binaryOperators[p::u8(op.type)]);
		AddExpr(inputs->linkedOutputs[1]);
		code->push_back(')');
	}

	void CGenerator::AddCast(ast::Id id)
	{
		const auto* inputs  = scope.TryGet<const ast::CExprInputs>(id);
		const auto* outputs = scope.TryGet<const ast::CExprOutputs>(id);
		if (!inputs || inputs->linkedOutputs.IsEmpty() || !outputs || outputs->pinIds.IsEmpty())
		{
			compiler.Error("Cast without a connected value");
			return;
		}

		const ast::Id sourceType = ast::GetPinType(scope, inputs->pinIds[0]);
		const ast::Id targetType = ast::GetPinType(scope, outputs->pinIds[0]);
		if (p::IsNone(sourceType) || p::IsNone(targetType))
		{
			compiler.Error("Cast between invalid types");
			return;
		}

		if (!ast::CastSystem::CanCast(scope, sourceType, targetType)) [[unlikely]]
		{
			compiler.Error(p::Format("'{}' can't cast into '{}'",
			    ast::GetName(compiler.ast, sourceType).AsString(),
			    ast::GetName(compiler.ast, targetType).AsString()));
			return;
		}

		const auto* irType = scope.TryGet<const CMIRType>(targetType);
		if (!irType) [[unlikely]]
		{
			compiler.Error("Cannot cast a value into an invalid type");
			return;
		}

		// Every cast is expressed as a C cast
		code->append("((");
		code->append(irType->value.AsString());
		code->append(")(");
		// The value is first cast into the type of the input pin
		AddExpr(inputs->linkedOutputs[0], inputs->pinIds[0]);
		code->append("))");
	}

	void CGenerator::AddStmtIf(ast::Id id)
	{
		const auto& outputs      = scope.Get<const ast::CStmtOutputs>(id);
		const auto& connectedIds = outputs.linkInputNodes;
		P_Check(connectedIds.Size() >= 1);
		const auto& exprInputs = scope.Get<const ast::CExprInputs>(id);
		P_Check(exprInputs.linkedOutputs.Size() == 1);

		code->append("if (");
		AddExpr(exprInputs.linkedOutputs.First(), exprInputs.pinIds[0]);
		code->append(") {\n");
		AddStmtBlock(connectedIds[0]);
		if (connectedIds.Size() > 1 && !p::IsNone(connectedIds[1]))
		{
			code->append("} else {\n");
			AddStmtBlock(connectedIds[1]);
		}
		code->append("}\n");
	}

	void CGenerator::AddStmtFor(ast::Id id)
	{
		const auto& outputs = scope.Get<const ast::CStmtOutputs>(id);
		P_Check(outputs.linkInputNodes.Size() >= 1);
		const auto& exprInputs = scope.Get<const ast::CExprInputs>(id);
		P_Check(exprInputs.linkedOutputs.Size() == 2);

		const ast::Id i32Id   = static_cast<ast::Tree&>(scope.GetContext()).GetNativeTypes().i32Id;
		const auto* indexType = scope.TryGet<const CMIRType>(i32Id);
		const p::StringView type = indexType ? indexType->value.AsString() : p::StringView{"long"};

		const auto& exprOutputs = scope.Get<const ast::CExprOutputs>(id);
		P_Check(!exprOutputs.pinIds.IsEmpty());
		const p::Tag indexTag =
		    scope.Get<const ast::CGeneratedName>(exprOutputs.pinIds.First()).name;
		if (indexTag.IsNone())
		{
			compiler.Error("For loop index has no generated name");
			return;
		}

		const p::StringView indexName = indexTag.AsString();
		p::String endName;
		p::FormatTo(endName, "{}_end", indexName);

		// The index stays visible in the continuation block after the loop
		p::FormatTo(*code, "{} {} = ", type, indexName);
		AddExpr(exprInputs.linkedOutputs[0], exprInputs.pinIds[0]);
		code->append(";\n{\n");
		p::FormatTo(*code, "{} {} = {} + ", type, endName, indexName);
		AddExpr(exprInputs.linkedOutputs[1], exprInputs.pinIds[1]);
		code->append(";\n");
		p::FormatTo(*code, "for (; {} < {}; ++{}) {{\n", indexName, endName, indexName);
		AddStmtBlock(outputs.linkInputNodes[0]);
		code->append("}\n}\n");

		if (outputs.linkInputNodes.Size() > 1 && !p::IsNone(outputs.linkInputNodes[1]))
		{
			AddStmtBlock(outputs.linkInputNodes[1]);
		}
	}

	void CGenerator::AddCall(ast::Id id, const ast::CExprCallId& call)
	{
		const ast::Id functionId = call.functionId;
		if (!scope.IsValid(functionId))
		{
			compiler.Error("Call to an unknown function");
			return;
		}
		if (!P_Ensure(scope.Has<CMIRFunctionSignature>(functionId)))
		{
			compiler.Error(p::Format(
			    "Call to an invalid function: '{}'", ast::GetFullName(scope, functionId)));
			return;
		}

		// Native C functions keep their original symbol name; Rift functions are
		// emitted module-qualified to avoid collisions.
		const ast::Id ownerId = p::GetIdParent(scope, functionId);
		if (scope.Has<CDeclCStruct>(ownerId) || scope.Has<CDeclCStatic>(ownerId))
		{
			code->append(ast::GetName(scope, functionId).AsString());
		}
		else
		{
			code->append(ast::GetFullName(scope, functionId, false, '_'));
		}
		code->push_back('(');

		if (const auto* inputs = scope.TryGet<const ast::CExprInputs>(id))
		{
			for (p::i32 i = 0; i < inputs->linkedOutputs.Size(); ++i)
			{
				const ast::ExprOutput output = inputs->linkedOutputs[i];
				if (!output.IsNone())
				{
					AddExpr(output, inputs->pinIds[i]);
					code->append(", ");
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

		p::FormatTo(*code, "int main() {{\n{}();\nreturn 0;\n}}\n",
		    ast::GetFullName(scope, functionId, false, '_'));
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
