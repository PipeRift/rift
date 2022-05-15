// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/TypeUtils.h"

#include "AST/Components/CDeclClass.h"
#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CDeclFunctionInterface.h"
#include "AST/Components/CDeclFunctionLibrary.h"
#include "AST/Components/CDeclStruct.h"
#include "AST/Components/CDeclVariable.h"
#include "AST/Components/CExprCall.h"
#include "AST/Components/CExprDeclRef.h"
#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprType.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CLiteralBool.h"
#include "AST/Components/CLiteralFloating.h"
#include "AST/Components/CLiteralIntegral.h"
#include "AST/Components/CLiteralString.h"
#include "AST/Components/CStmtIf.h"
#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Components/CStmtReturn.h"
#include "AST/Serialization.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/Hierarchy.h"
#include "AST/Utils/Paths.h"
#include "AST/Utils/Statements.h"
#include "AST/Utils/TransactionUtils.h"

#include <Files/Files.h>
#include <Misc/Checks.h>
#include <Profiler.h>
#include <Serialization/Formats/JsonFormat.h>


namespace Rift::Types
{
	void InitTypeFromCategory(AST::Tree& ast, AST::Id id, Type category)
	{
		if (auto* fileRef = ast.TryGet<CFileRef>(id))
		{
			String fileName = Paths::GetFilename(fileRef->path);
			fileName        = Strings::RemoveFromEnd(fileName, Paths::typeExtension);
			ast.Add<CType>(id, {Name{fileName}});
		}

		switch (category)
		{
			case Type::Class: ast.Add<CDeclClass>(id); break;
			case Type::Struct: ast.Add<CDeclStruct>(id); break;
			case Type::FunctionLibrary: ast.Add<CDeclFunctionLibrary>(id); break;
			case Type::FunctionInterface: ast.Add<CDeclFunctionInterface>(id); break;
			default: break;
		}
	}

	Type GetCategory(AST::Tree& ast, AST::Id id)
	{
		if (ast.Has<CDeclStruct>(id))
		{
			return Type::Struct;
		}
		else if (ast.Has<CDeclClass>(id))
		{
			return Type::Class;
		}
		else if (ast.Has<CDeclFunctionLibrary>(id))
		{
			return Type::FunctionLibrary;
		}
		else if (ast.Has<CDeclFunctionInterface>(id))
		{
			return Type::FunctionInterface;
		}
		return Type::None;
	}

	AST::Id CreateType(AST::Tree& ast, Type type, Name name, const Path& path)
	{
		AST::Id id = ast.Create();
		if (!name.IsNone())
		{
			ast.Add<CIdentifier>(id, name);
		}
		if (!path.empty())
		{
			ast.Add<CFileRef>(id, path);
		}
		InitTypeFromCategory(ast, id, type);
		return id;
	}

	void RemoveTypes(TAccessRef<TWrite<CChild>, TWrite<CParent>, CFileRef> access,
	    TSpan<AST::Id> typeIds, bool removeFromDisk)
	{
		if (removeFromDisk)
		{
			for (AST::Id id : typeIds)
			{
				if (const auto* file = access.TryGet<const CFileRef>(id))
				{
					Files::Delete(file->path, true, false);
				}
			}
		}
		AST::Hierarchy::RemoveDeep(access, typeIds);
	}

	void Serialize(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;

		Serl::JsonFormatWriter writer{};
		AST::WriteContext ct{writer.GetContext(), ast, true};
		ct.BeginObject();
		ct.Next("type", GetCategory(ast, id));
		ct.SerializeEntity(id);
		data = writer.ToString();
	}

	void Deserialize(AST::Tree& ast, AST::Id id, const String& data)
	{
		ZoneScoped;

		Serl::JsonFormatReader reader{data};
		if (!reader.IsValid())
		{
			return;
		}

		AST::ReadContext ct{reader, ast};
		ct.BeginObject();

		Type category = Type::None;
		ct.Next("type", category);
		Types::InitTypeFromCategory(ast, id, category);

		ct.SerializeEntity(id);
	}


	AST::Id FindTypeByPath(AST::Tree& ast, const Path& path)
	{
		if (auto* types = ast.TryGetStatic<STypes>())
		{
			const Name pathName{Paths::ToString(path)};
			if (AST::Id* id = types->typesByPath.Find(pathName))
			{
				return *id;
			}
		}
		return AST::NoId;
	}

	bool IsClass(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CDeclClass>(typeId);
	}

	bool IsStruct(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CDeclStruct>(typeId);
	}

	bool IsFunctionLibrary(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.Has<CDeclFunctionLibrary>(typeId);
	}

	bool CanContainVariables(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.HasAny<CDeclClass, CDeclStruct>(typeId);
	}

	bool CanContainFunctions(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.HasAny<CDeclClass, CDeclFunctionLibrary>(typeId);
	}


	AST::Id AddVariable(AST::TypeRef type, Name name)
	{
		AST::Tree& ast = type.GetAST();

		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CDeclVariable, CParent>(id);

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type, id);
		}
		return id;
	}

	AST::Id AddFunction(AST::TypeRef type, Name name)
	{
		AST::Tree& ast = type.GetAST();

		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CDeclFunction, CParent>(id);
		ast.Add<CStmtOutput>(id);

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type, id);
		}
		return id;
	}

	AST::Id AddCall(AST::TypeRef type, AST::Id functionId)
	{
		AST::Tree& ast       = type.GetAST();
		const AST::Id callId = ast.Create();

		auto& callExprId      = ast.Add<CExprCallId>(callId);
		callExprId.functionId = functionId;
		auto& callExpr        = ast.Add<CExprCall>(callId);
		callExpr.functionName = ast.Get<CIdentifier>(functionId).name;
		ast.Add<CStmtInput, CStmtOutput, CExprOutputs, CExprInputs>(callId);

		const AST::Id typeId = AST::Hierarchy::GetParent(ast, functionId);
		if (!IsNone(typeId))
		{
			callExpr.ownerName = ast.Get<CType>(typeId).name;
		}

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), callId);
		}
		return callId;
	}

	AST::Id AddCallInput(AST::Tree& ast, AST::Id functionId, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CExprType>(id);
		AST::Hierarchy::AddChildren(ast, functionId, id);
		ast.GetOrAdd<CExprOutputs>(functionId).Add(id);
		return id;
	}

	AST::Id AddCallOutput(AST::Tree& ast, AST::Id functionId, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CIdentifier>(id, name);
		ast.Add<CExprType>(id);
		AST::Hierarchy::AddChildren(ast, functionId, id);
		ast.GetOrAdd<CExprInputs>(functionId).Add(id);
		return id;
	}

	void GetCallArgs(AST::Tree& ast, TSpan<AST::Id> callIds, TArray<AST::Id>& inputArgIds,
	    TArray<AST::Id>& outputArgIds, TArray<AST::Id>& otherIds)
	{
		auto exprInputs  = ast.Filter<CExprInputs>();
		auto exprOutputs = ast.Filter<CExprOutputs>();

		TArray<AST::Id> children;
		for (AST::Id id : callIds)
		{
			AST::Hierarchy::GetChildren(ast, id, children);
		}

		for (AST::Id id : children)
		{
			if (exprInputs.Has(id))
			{
				inputArgIds.Add(id);
			}
			else if (exprOutputs.Has(id))
			{
				outputArgIds.Add(id);
			}
			else
			{
				otherIds.Add(id);
			}
		}
	}

	AST::Id AddIf(AST::TypeRef type)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();
		ast.Add<CStmtIf>(id);
		ast.Add<CStmtInput>(id);

		// Bool input
		const AST::Id valueId = ast.Create();
		ast.Add<CExprType>(valueId, {ast.GetNativeTypes().boolId});
		AST::Hierarchy::AddChildren(ast, id, valueId);
		ast.Add<CExprInputs>(id).Add(valueId);

		TArray<AST::Id> outIds(2);
		ast.Create(outIds);
		AST::Hierarchy::AddChildren(ast, id, outIds);
		ast.Add<CStmtOutputs>(id, Move(outIds));

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id AddReturn(AST::TypeRef type)
	{
		AST::Tree& ast         = type.GetAST();
		const AST::Id returnId = ast.Create();
		ast.Add<CStmtReturn>(returnId);
		ast.Add<CStmtInput>(returnId);
		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), returnId);
		}
		return returnId;
	}

	AST::Id AddLiteral(AST::TypeRef type, AST::Id literalTypeId)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();
		ast.Add<CExprType>(id, {literalTypeId});
		ast.Add<CExprOutputs>(id).Add(id);

		bool created        = false;
		const auto& natives = ast.GetNativeTypes();
		// TODO: Refactor this
		if (literalTypeId == natives.boolId)
		{
			ast.Add<CLiteralBool>(id);
			created = true;
		}
		else if (literalTypeId == natives.floatId)
		{
			ast.Add<CLiteralFloating>(id).type = FloatingType::F32;
			created                            = true;
		}
		else if (literalTypeId == natives.doubleId)
		{
			ast.Add<CLiteralFloating>(id).type = FloatingType::F64;
			created                            = true;
		}
		else if (literalTypeId == natives.u8Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::U8;
			created                            = true;
		}
		else if (literalTypeId == natives.u16Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::U16;
			created                            = true;
		}
		else if (literalTypeId == natives.u32Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::U32;
			created                            = true;
		}
		else if (literalTypeId == natives.u64Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::U64;
			created                            = true;
		}
		else if (literalTypeId == natives.i8Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::S8;
			created                            = true;
		}
		else if (literalTypeId == natives.i16Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::S16;
			created                            = true;
		}
		else if (literalTypeId == natives.i32Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::S32;
			created                            = true;
		}
		else if (literalTypeId == natives.i64Id)
		{
			ast.Add<CLiteralIntegral>(id).type = IntegralType::S64;
			created                            = true;
		}
		else if (literalTypeId == natives.stringId)
		{
			ast.Add<CLiteralString>(id);
			created = true;
		}

		if (!created)
		{
			ast.Destroy(id);
			return AST::NoId;
		}

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id AddDeclarationReference(AST::TypeRef type, AST::Id declId)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();

		ast.Add<CExprDeclRef>(id);
		ast.Add<CExprOutputs>(id).Add(id);    // Types gets resolved by a system later

		const AST::Id typeId = AST::Hierarchy::GetParent(ast, declId);
		Check(!IsNone(typeId));
		auto& declRefExpr           = ast.Add<CExprDeclRef>(id);
		declRefExpr.ownerName       = ast.Get<CType>(typeId).name;
		declRefExpr.name            = ast.Get<CIdentifier>(declId).name;
		auto& declRefExprId         = ast.Add<CExprDeclRefId>(id);
		declRefExprId.declarationId = declId;

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id AddUnaryOperator(AST::TypeRef type, UnaryOperatorType operatorType)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();
		ast.Add<CExprUnaryOperator>(id, {operatorType});
		ast.Add<CExprInputs>(id).Add(id);
		ast.Add<CExprOutputs>(id).Add(id);
		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id AddBinaryOperator(AST::TypeRef type, BinaryOperatorType operatorType)
	{
		AST::Tree& ast   = type.GetAST();
		const AST::Id id = ast.Create();
		ast.Add<CExprBinaryOperator>(id, {operatorType});
		ast.Add<CExprOutputs>(id);

		auto& inputs = ast.Add<CExprInputs>(id);
		inputs.Resize(2);
		ast.Create(inputs.pinIds);
		AST::Hierarchy::AddChildren(ast, id, inputs.pinIds);
		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	AST::Id FindFunctionByName(AST::Tree& ast, Name ownerName, Name functionName)
	{
		auto& types = ast.GetStatic<STypes>();
		if (const AST::Id* typeId = types.typesByName.Find(ownerName))
		{
			if (const auto* children = AST::Hierarchy::GetChildren(ast, *typeId))
			{
				auto functions = ast.Filter<CDeclFunction, CIdentifier>();
				for (AST::Id childId : *children)
				{
					if (functions.Has(childId)
					    && functions.Get<CIdentifier>(childId).name == functionName)
					{
						return childId;
					}
				}
			}
		}
		return AST::NoId;
	}

	void RemoveNodes(const RemoveAccess& access, TSpan<AST::Id> ids)
	{
		ScopedChange(access, ids);
		AST::Hierarchy::RemoveDeep(access, ids);
	}
}    // namespace Rift::Types
