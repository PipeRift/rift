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
#include "AST/Components/Views/CNodePosition.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/Namespaces.h"
#include "AST/Utils/Paths.h"
#include "AST/Utils/Statements.h"
#include "AST/Utils/TransactionUtils.h"

#include <Pipe/Core/Checks.h>
#include <Pipe/Core/Profiler.h>
#include <Pipe/ECS/Serialization.h>
#include <Pipe/ECS/Utils/Hierarchy.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Serialize/Formats/JsonFormat.h>


namespace rift::AST::Types
{
	auto typeComponents = [](auto& rw) {
		rw.template SerializeComponents<CChild, CDeclVariable, CDeclFunction, CExprBinaryOperator,
		    CExprCall, CExprDeclRefId, CExprOutputs, CExprInputs, CStmtReturn, CExprType,
		    CExprUnaryOperator, CNodePosition, CNamespace, CParent, CLiteralBool, CLiteralFloating,
		    CLiteralIntegral, CLiteralString, CStmtIf, CStmtOutput, CStmtOutputs, CStmtInput>();
	};

	void InitTypeFromCategory(Tree& ast, Id id, RiftType category)
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

	RiftType GetCategory(Tree& ast, Id id)
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

	Id CreateType(Tree& ast, RiftType type, Name name, const p::Path& path)
	{
		Id id = ast.Create();
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
	    TSpan<Id> typeIds, bool removeFromDisk)
	{
		if (removeFromDisk)
		{
			for (Id id : typeIds)
			{
				if (const auto* file = access.TryGet<const CFileRef>(id))
				{
					files::Delete(file->path, true, false);
				}
			}
		}
		p::ecs::RemoveDeep(access, typeIds);
	}

	void Serialize(Tree& ast, Id id, String& data)
	{
		ZoneScoped;

		JsonFormatWriter writer{};
		p::ecs::EntityWriter w{writer.GetContext(), ast};
		w.BeginObject();
		w.Next("type", GetCategory(ast, id));
		w.SerializeEntity(id, typeComponents);

		data = writer.ToString();
	}

	void Deserialize(Tree& ast, Id id, const String& data)
	{
		ZoneScoped;

		JsonFormatReader reader{data};
		if (!reader.IsValid())
		{
			return;
		}

		p::ecs::EntityReader r{reader, ast};
		r.BeginObject();

		RiftType category = RiftType::None;
		r.Next("type", category);
		Types::InitTypeFromCategory(ast, id, category);

		r.SerializeEntity(id, typeComponents);
	}


	Id FindTypeByPath(Tree& ast, const p::Path& path)
	{
		if (auto* types = ast.TryGetStatic<STypes>())
		{
			const Name pathName{ToString(path)};
			if (Id* id = types->typesByPath.Find(pathName))
			{
				return *id;
			}
		}
		return NoId;
	}

	bool IsClass(const Tree& ast, Id typeId)
	{
		return ast.Has<CDeclClass>(typeId);
	}

	bool IsStruct(const Tree& ast, Id typeId)
	{
		return ast.Has<CDeclStruct>(typeId);
	}

	bool IsFunctionLibrary(const Tree& ast, Id typeId)
	{
		return ast.Has<CDeclFunctionLibrary>(typeId);
	}

	bool CanContainVariables(const Tree& ast, Id typeId)
	{
		return ast.HasAny<CDeclClass, CDeclStruct>(typeId);
	}

	bool CanContainFunctions(const Tree& ast, Id typeId)
	{
		return ast.HasAny<CDeclClass, CDeclFunctionLibrary, CDeclFunctionInterface>(typeId);
	}

	bool CanEditFunctionBodies(const Tree& ast, Id typeId)
	{
		return ast.HasAny<CDeclClass, CDeclFunctionLibrary>(typeId);
	}


	Id AddVariable(TypeRef type, Name name)
	{
		Tree& ast = type.GetContext();

		Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CDeclVariable, CParent>(id);

		if (type)
		{
			p::ecs::AddChildren(ast, type, id);
		}
		return id;
	}

	Id AddFunction(TypeRef type, Name name)
	{
		Tree& ast = type.GetContext();

		Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CDeclFunction, CParent>(id);
		ast.Add<CStmtOutput>(id);

		if (type)
		{
			p::ecs::AddChildren(ast, type, id);
		}
		return id;
	}

	Id AddCall(TypeRef type, Id functionId)
	{
		Tree& ast   = type.GetContext();
		const Id id = ast.Create();

		ast.Add<CStmtInput, CStmtOutput, CExprOutputs, CExprInputs>(id);

		ast.Add<CExprCallId>(id, {functionId});
		ast.Add<CExprCall>(id).function = GetNamespace(ast, functionId);

		if (type)
		{
			p::ecs::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	Id AddFunctionInput(Tree& ast, Id functionId, Name name)
	{
		Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CExprTypeId>(id);
		ast.Add<CExprType>(id);
		p::ecs::AddChildren(ast, functionId, id);
		ast.GetOrAdd<CExprOutputs>(functionId).Add(id);
		return id;
	}

	Id AddFunctionOutput(Tree& ast, Id functionId, Name name)
	{
		Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CExprTypeId>(id);
		ast.Add<CExprType>(id);
		p::ecs::AddChildren(ast, functionId, id);
		ast.GetOrAdd<CExprInputs>(functionId).Add(id);
		return id;
	}

	Id AddIf(TypeRef type)
	{
		Tree& ast   = type.GetContext();
		const Id id = ast.Create();
		ast.Add<CStmtIf>(id);
		ast.Add<CStmtInput>(id);

		// Bool input
		const Id valueId = ast.Create();
		ast.Add<CExprTypeId>(valueId, {.id = ast.GetNativeTypes().boolId});
		ast.Add<CExprType>(id).type = GetNamespace(ast, ast.GetNativeTypes().boolId);
		p::ecs::AddChildren(ast, id, valueId);
		ast.Add<CExprInputs>(id).Add(valueId);

		TArray<Id> outIds(2);
		ast.Create(outIds);
		p::ecs::AddChildren(ast, id, outIds);
		ast.Add<CStmtOutputs>(id, Move(outIds));

		if (type)
		{
			p::ecs::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	Id AddReturn(TypeRef type)
	{
		Tree& ast         = type.GetContext();
		const Id returnId = ast.Create();
		ast.Add<CStmtReturn>(returnId);
		ast.Add<CStmtInput>(returnId);
		if (type)
		{
			p::ecs::AddChildren(ast, type.GetId(), returnId);
		}
		return returnId;
	}

	Id AddLiteral(TypeRef type, Id literalTypeId)
	{
		Tree& ast   = type.GetContext();
		const Id id = ast.Create();
		ast.Add<CExprTypeId>(id, {.id = literalTypeId});
		ast.Add<CExprType>(id).type = GetNamespace(ast, literalTypeId);
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
			return NoId;
		}

		if (type)
		{
			p::ecs::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	Id AddDeclarationReference(TypeRef type, Id declId)
	{
		Tree& ast   = type.GetContext();
		const Id id = ast.Create();

		ast.Add<CExprDeclRef>(id);
		ast.Add<CExprOutputs>(id).Add(id);    // Types gets resolved by a system later

		const Id typeId = p::ecs::GetParent(ast, declId);
		Check(!IsNone(typeId));
		auto& declRefExpr           = ast.Add<CExprDeclRef>(id);
		declRefExpr.ownerName       = ast.Get<CNamespace>(typeId).name;
		declRefExpr.name            = ast.Get<CNamespace>(declId).name;
		auto& declRefExprId         = ast.Add<CExprDeclRefId>(id);
		declRefExprId.declarationId = declId;

		if (type)
		{
			p::ecs::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	Id AddUnaryOperator(TypeRef type, UnaryOperatorType operatorType)
	{
		Tree& ast   = type.GetContext();
		const Id id = ast.Create();
		ast.Add<CExprUnaryOperator>(id, {operatorType});
		ast.Add<CExprInputs>(id).Add(id);
		ast.Add<CExprOutputs>(id).Add(id);
		if (type)
		{
			p::ecs::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	Id AddBinaryOperator(TypeRef type, BinaryOperatorType operatorType)
	{
		Tree& ast   = type.GetContext();
		const Id id = ast.Create();
		ast.Add<CExprBinaryOperator>(id, {operatorType});
		ast.Add<CExprOutputs>(id).Add(id);

		auto& inputs = ast.Add<CExprInputs>(id);
		inputs.Resize(2);
		ast.Create(inputs.pinIds);
		p::ecs::AddChildren(ast, id, inputs.pinIds);
		if (type)
		{
			p::ecs::AddChildren(ast, type.GetId(), id);
		}
		return id;
	}

	Id FindChildByName(TAccessRef<CNamespace, CParent> access, Id ownerId, Name functionName)
	{
		if (!IsNone(ownerId))
		{
			TArray<Id> children;
			p::ecs::GetChildren(access, ownerId, children);
			for (Id childId : children)
			{
				const auto* ns = access.TryGet<const CNamespace>(childId);
				if (ns && ns->name == functionName)
				{
					return childId;
				}
			}
		}
		return NoId;
	}

	void RemoveNodes(const RemoveAccess& access, TSpan<Id> ids)
	{
		ScopedChange(access, ids);
		p::ecs::RemoveDeep(access, ids);
	}

	bool CopyExpressionType(TAccessRef<TWrite<CExprTypeId>> access, Id sourcePinId, Id targetPinId)
	{
		auto* sourceType = access.TryGet<const CExprTypeId>(sourcePinId);
		auto* targetType = access.TryGet<CExprTypeId>(targetPinId);
		if (!sourceType || IsNone(sourceType->id)
		    || (targetType && sourceType->id == targetType->id))
		{
			return false;
		}

		if (targetType)
		{
			*targetType = *sourceType;
		}
		else
		{
			access.Add<CExprTypeId>(targetPinId, *sourceType);
		}
		return true;
	}
}    // namespace rift::AST::Types
