// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/TypeUtils.h"

#include "AST/Components/CDeclClass.h"
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
#include "AST/Components/CLiteralBool.h"
#include "AST/Components/CLiteralFloating.h"
#include "AST/Components/CLiteralIntegral.h"
#include "AST/Components/CLiteralString.h"
#include "AST/Components/CNamespace.h"
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

#include <Pipe/Core/Checks.h>
#include <Pipe/Core/Profiler.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Serialize/Formats/JsonFormat.h>


namespace rift::Types
{
	void InitTypeFromCategory(AST::Tree& ast, AST::Id id, RiftType category)
	{
		if (auto* fileRef = ast.TryGet<CFileRef>(id))
		{
			String fileName = p::GetFilename(fileRef->path);
			fileName        = Strings::RemoveFromEnd(fileName, Paths::typeExtension);
			ast.Add<CNamespace>(id, {Name{fileName}});
		}

		ast.Add<CType>(id);

		switch (category)
		{
			case RiftType::Class: ast.Add<CDeclClass>(id); break;
			case RiftType::Struct: ast.Add<CDeclStruct>(id); break;
			case RiftType::FunctionLibrary: ast.Add<CDeclFunctionLibrary>(id); break;
			case RiftType::FunctionInterface: ast.Add<CDeclFunctionInterface>(id); break;
			default: break;
		}
	}

	RiftType GetCategory(AST::Tree& ast, AST::Id id)
	{
		if (ast.Has<CDeclStruct>(id))
		{
			return RiftType::Struct;
		}
		else if (ast.Has<CDeclClass>(id))
		{
			return RiftType::Class;
		}
		else if (ast.Has<CDeclFunctionLibrary>(id))
		{
			return RiftType::FunctionLibrary;
		}
		else if (ast.Has<CDeclFunctionInterface>(id))
		{
			return RiftType::FunctionInterface;
		}
		return RiftType::None;
	}

	AST::Id CreateType(AST::Tree& ast, RiftType type, Name name, const p::Path& path)
	{
		AST::Id id = ast.Create();
		if (!path.empty())
		{
			ast.Add<CFileRef>(id, path);
		}
		InitTypeFromCategory(ast, id, type);

		if (!name.IsNone() && !ast.Has<CNamespace>(id))
		{
			ast.Add<CNamespace>(id, name);
		}
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
					files::Delete(file->path, true, false);
				}
			}
		}
		AST::Hierarchy::RemoveDeep(access, typeIds);
	}

	void Serialize(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;

		JsonFormatWriter writer{};
		AST::Writer ct{writer.GetContext(), ast, true};
		ct.BeginObject();
		ct.Next("type", GetCategory(ast, id));
		ct.SerializeEntity(id);
		data = writer.ToString();
	}

	void Deserialize(AST::Tree& ast, AST::Id id, const String& data)
	{
		ZoneScoped;

		JsonFormatReader reader{data};
		if (!reader.IsValid())
		{
			return;
		}

		AST::Reader ct{reader, ast};
		ct.BeginObject();

		RiftType category = RiftType::None;
		ct.Next("type", category);
		Types::InitTypeFromCategory(ast, id, category);

		ct.SerializeEntity(id);
	}


	AST::Id FindTypeByPath(AST::Tree& ast, const p::Path& path)
	{
		if (auto* types = ast.TryGetStatic<STypes>())
		{
			const Name pathName{ToString(path)};
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
		return ast.HasAny<CDeclClass, CDeclFunctionLibrary, CDeclFunctionInterface>(typeId);
	}

	bool CanEditFunctionBodies(const AST::Tree& ast, AST::Id typeId)
	{
		return ast.HasAny<CDeclClass, CDeclFunctionLibrary>(typeId);
	}


	AST::Id AddVariable(AST::TypeRef type, Name name)
	{
		AST::Tree& ast = type.GetContext();

		AST::Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CDeclVariable, CParent>(id);

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type, id);
		}
		return id;
	}

	AST::Id AddFunction(AST::TypeRef type, Name name)
	{
		AST::Tree& ast = type.GetContext();

		AST::Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
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
		AST::Tree& ast       = type.GetContext();
		const AST::Id callId = ast.Create();

		auto& callExprId      = ast.Add<CExprCallId>(callId);
		callExprId.functionId = functionId;
		auto& callExpr        = ast.Add<CExprCall>(callId);
		callExpr.functionName = ast.Get<CNamespace>(functionId).name;
		ast.Add<CStmtInput, CStmtOutput, CExprOutputs, CExprInputs>(callId);

		const AST::Id typeId = AST::Hierarchy::GetParent(ast, functionId);
		if (!IsNone(typeId))
		{
			callExpr.ownerName = ast.Get<const CNamespace>(typeId).name;
		}

		if (type)
		{
			AST::Hierarchy::AddChildren(ast, type.GetId(), callId);
		}
		return callId;
	}

	AST::Id AddFunctionInput(AST::Tree& ast, AST::Id functionId, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CExprType>(id);
		AST::Hierarchy::AddChildren(ast, functionId, id);
		ast.GetOrAdd<CExprOutputs>(functionId).Add(id);
		return id;
	}

	AST::Id AddFunctionOutput(AST::Tree& ast, AST::Id functionId, Name name)
	{
		AST::Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CExprType>(id);
		AST::Hierarchy::AddChildren(ast, functionId, id);
		ast.GetOrAdd<CExprInputs>(functionId).Add(id);
		return id;
	}

	AST::Id AddIf(AST::TypeRef type)
	{
		AST::Tree& ast   = type.GetContext();
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
		AST::Tree& ast         = type.GetContext();
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
		AST::Tree& ast   = type.GetContext();
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
		AST::Tree& ast   = type.GetContext();
		const AST::Id id = ast.Create();

		ast.Add<CExprDeclRef>(id);
		ast.Add<CExprOutputs>(id).Add(id);    // Types gets resolved by a system later

		const AST::Id typeId = AST::Hierarchy::GetParent(ast, declId);
		Check(!IsNone(typeId));
		auto& declRefExpr           = ast.Add<CExprDeclRef>(id);
		declRefExpr.ownerName       = ast.Get<CNamespace>(typeId).name;
		declRefExpr.name            = ast.Get<CNamespace>(declId).name;
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
		AST::Tree& ast   = type.GetContext();
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
		AST::Tree& ast   = type.GetContext();
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

	AST::Id FindFunctionByName(
	    TAccessRef<CDeclFunction, CNamespace, CParent> access, Name ownerName, Name functionName)
	{
		auto& types = access.GetContext().GetStatic<STypes>();
		if (const AST::Id* typeId = types.typesByName.Find(ownerName))
		{
			return FindFunctionByName(access, *typeId, functionName);
		}
		return AST::NoId;
	}

	AST::Id FindFunctionByName(
	    TAccessRef<CDeclFunction, CNamespace, CParent> access, AST::Id ownerId, Name functionName)
	{
		if (!IsNone(ownerId))
		{
			TArray<AST::Id> children;
			AST::Hierarchy::GetChildren(access, ownerId, children);
			ecs::ExcludeIfNot<CDeclFunction, CNamespace>(access, children);
			for (AST::Id childId : children)
			{
				if (access.Get<const CNamespace>(childId).name == functionName)
				{
					return childId;
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
}    // namespace rift::Types
