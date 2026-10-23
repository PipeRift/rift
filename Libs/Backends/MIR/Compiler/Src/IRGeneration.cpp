// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "IRGeneration.h"

#include <AST/Components/Declarations.h>
#include <AST/Systems/CastSystem.h>
#include <AST/Systems/FunctionsSystem.h>
#include <AST/Utils/Expressions.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Namespaces.h>
#include <AST/Utils/Statements.h>
#include <Compiler/Compiler.h>
#include <Pipe/Core/Map.h>
#include <Pipe/Core/StringView.h>
#include <PipeStrings.h>


namespace rift::MIR
{
	const p::TSet<p::Tag> CGenerator::reservedNames{"class", "struct", "if", "for"};

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
		struct NameScopeStack
		{
			using NameScope = p::TMap<p::Tag, p::i32>;

			NameScope current;
			p::TArray<NameScope> scopes;

			void PushScope()
			{
				scopes.Add(current);
			}

			void PopScope()
			{
				P_Check(scopes.Size() > 0);
				current = scopes.Last();
				scopes.RemoveLast();
			}

			p::Tag AssignName(p::Tag base)
			{
				if (base.IsNone())
				{
					return p::Tag{};
				}

				const p::StringView baseName = base.AsString();

				p::i32 suffix = 0;
				auto it       = current.FindIt(base);
				if (it == current.end())
				{
					current.Insert(base, 0);
					return p::Tag{baseName};
				}

				suffix         = ++(const_cast<p::i32&>(it->second));
				p::String name = p::Format("{}_{}", baseName, suffix);

				while (true)
				{
					const p::Tag tag{name};
					if (!current.Contains(tag))
					{
						current.Insert(tag, 0);
						return tag;
					}

					// A variable with this name already exists, try the next suffix
					name.clear();
					p::FormatTo(name, "{}_{}", baseName, suffix);
					suffix = ++(const_cast<p::i32&>(it->second));
				}
			}
		};

		void AssignVariables(ast::Tree& ast, ast::Id ownerId, NameScopeStack& names)
		{
			p::TArray<ast::Id> children;
			p::GetIdChildren(ast, ownerId, children);
			for (ast::Id childId : children)
			{
				if (ast.Has<ast::CDeclVariable>(childId))
				{
					ast.Add<ast::CGeneratedName>(
					    childId, {names.AssignName(ast::GetName(ast, childId))});
				}
			}
		}

		void AssignStatementNames(ast::Tree& ast, ast::Id nodeId, NameScopeStack& names)
		{
			auto assignPins = [&](const p::TArray<p::Id>& pinIds)
			{
				for (p::Id pinId : pinIds)
				{
					ast.Add<ast::CGeneratedName>(
					    pinId, {names.AssignName(ast::GetName(ast, pinId))});
				}
			};

			// Function inputs and for-loop indices live in CExprOutputs
			if (const auto* outputs = ast.TryGet<const ast::CExprOutputs>(nodeId))
			{
				assignPins(outputs->pinIds);
			}
			if (const auto* inputs = ast.TryGet<const ast::CExprInputs>(nodeId))
			{
				assignPins(inputs->pinIds);
			}
		}

		void AssignFunctionNames(ast::Tree& ast, ast::Id functionId, NameScopeStack& names,
		    const p::TMap<ast::Id, p::TArray<ast::Id>>& stmtIdsByFunction)
		{
			names.PushScope();

			// Function parameters and outputs
			AssignStatementNames(ast, functionId, names);

			// Locals declared directly in the function share its scope
			AssignVariables(ast, functionId, names);

			// Every statement in the function body names its expression pins here.
			// Loop indices land in the function scope; statement-local variables get
			// their own nested scope.
			if (const auto* stmtIds = stmtIdsByFunction.Find(functionId))
			{
				for (ast::Id stmtId : *stmtIds)
				{
					AssignStatementNames(ast, stmtId, names);

					names.PushScope();
					AssignVariables(ast, stmtId, names);
					names.PopScope();
				}
			}

			names.PopScope();
		}
	}    // namespace

	void CGenerator::AssignGeneratedNames()
	{
		ast::Tree& ast = compiler.ast;
		ast.ClearPool<ast::CGeneratedName>();

		NameScopeStack names;

		// Globals come first so parameters and locals can avoid them
		for (ast::Id typeId : p::FindAllIdsWith<ast::CDeclStatic>(ast))
		{
			AssignVariables(ast, typeId, names);
		}

		// Statement nodes assign names to their expression pins in the owning
		// function's scope.
		p::TArray<ast::Id> stmtIds = p::FindAllIdsWith<ast::CStmtInput>(ast);
		stmtIds.Sort();
		p::TMap<ast::Id, p::TArray<ast::Id>> stmtIdsByFunction;
		for (ast::Id stmtId : stmtIds)
		{
			const ast::Id functionId = ast::FunctionsSystem::FindOwningFunction(ast, stmtId);
			if (!p::IsNone(functionId))
			{
				stmtIdsByFunction[functionId].Add(stmtId);
			}
		}

		p::TArray<ast::Id> functionIds = p::FindAllIdsWith<ast::CDeclFunction>(ast);
		p::TMap<ast::Id, p::TArray<ast::Id>> functionsByType;
		for (ast::Id functionId : functionIds)
		{
			functionsByType[p::GetIdParent(ast, functionId)].Add(functionId);
		}

		// Struct and class members share the type scope with their functions
		p::TArray<ast::Id> memberTypeIds;
		memberTypeIds.Append(p::FindAllIdsWith<ast::CDeclStruct>(ast));
		memberTypeIds.Append(p::FindAllIdsWith<ast::CDeclClass>(ast));
		for (ast::Id typeId : memberTypeIds)
		{
			names.PushScope();
			AssignVariables(ast, typeId, names);
			if (const auto* functions = functionsByType.Find(typeId))
			{
				for (ast::Id functionId : *functions)
				{
					AssignFunctionNames(ast, functionId, names, stmtIdsByFunction);
				}
			}
			names.PopScope();
		}

		// Every other function starts from the global scope
		for (ast::Id functionId : functionIds)
		{
			const ast::Id ownerId = p::GetIdParent(ast, functionId);
			if (ast.Has<ast::CDeclStruct>(ownerId) || ast.Has<ast::CDeclClass>(ownerId))
			{
				continue;
			}

			AssignFunctionNames(ast, functionId, names, stmtIdsByFunction);
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
			else if (scope.Has<const ast::CStmtAssign>(id))
			{
				// Not generated: report instead of dropping the statement
				compiler.Error("Variable assignments are not supported yet");
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
		else if (scope.Has<const ast::CStmtAssign>(output.nodeId))
		{
			compiler.Error("Variable assignments are not supported yet");
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
		const p::StringView type = indexType->value.AsString();

		const auto& exprOutputs = scope.Get<const ast::CExprOutputs>(id);
		P_Check(exprOutputs.pinIds.Size() == 1);
		const p::Tag indexTag = scope.Get<const ast::CGeneratedName>(exprOutputs.pinIds[0]).name;

		// The end expression is cached in a local named after its input pin
		const p::Tag endTag = scope.Get<const ast::CGeneratedName>(exprInputs.pinIds[1]).name;

		const p::StringView indexName = indexTag.AsString();
		const p::StringView endName   = endTag.AsString();

		p::FormatTo(*code, "for ({0} {1}=", type, indexName);
		AddExpr(exprInputs.linkedOutputs[0], exprInputs.pinIds[0]);
		p::FormatTo(*code, ", {}=", endName);
		AddExpr(exprInputs.linkedOutputs[1], exprInputs.pinIds[1]);
		p::FormatTo(*code, "; {0} < {1}; ++{0}) {{\n", indexName, endName);
		AddStmtBlock(outputs.linkInputNodes[0]);
		code->append("}\n");

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
