// Copyright 2015-2023 Piperift - All rights reserved

#include "AST/Utils/TypeUtils.h"

#include "AST/Components/CFileRef.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/Declarations.h"
#include "AST/Components/Expressions.h"
#include "AST/Components/Literals.h"
#include "AST/Components/Statements.h"
#include "AST/Components/Views/CNodePosition.h"
#include "AST/Statics/STypes.h"
#include "AST/Utils/Namespaces.h"
#include "AST/Utils/Paths.h"
#include "AST/Utils/TransactionUtils.h"
#include "ASTModule.h"
#include "Rift.h"

#include <Pipe/Core/Checks.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Serialize/Formats/JsonFormat.h>
#include <PipeECS.h>


namespace rift::ast
{
	static p::TArray<RiftType> gFileTypes;

	auto gTypeComponents = [](auto& rw) {
		rw.template SerializePools<CChild, CDeclVariable, CDeclFunction, CExprBinaryOperator,
		    CExprCall, CExprDeclRefId, CExprOutputs, CExprInputs, CStmtReturn, CExprType,
		    CExprUnaryOperator, CNodePosition, CNamespace, CParent, CLiteralBool, CLiteralFloating,
		    CLiteralIntegral, CLiteralString, CStmtIf, CStmtOutput, CStmtOutputs, CStmtInput>();
	};

	void InitTypeFromFileType(Tree& ast, Id id, p::Tag typeId)
	{
		if (auto* fileRef = ast.TryGet<CFileRef>(id))
		{
			p::StringView fileName{p::GetFilename(fileRef->path)};
			fileName = Strings::RemoveFromEnd(fileName, Paths::typeExtension);
			ast.Add<CNamespace>(id, {Tag{fileName}});
		}

		ast.Add<CDeclType>(id, {.typeId = typeId});

		if (auto* fileType = FindFileType(typeId))
		{
			ast.AddDefault(fileType->tagType->GetId(), id);
		}
	}

	Id CreateType(Tree& ast, p::Tag typeId, Tag name, StringView path)
	{
		Id id = ast.Create();
		if (!path.empty())
		{
			ast.Add<CFileRef>(id, path);
		}
		InitTypeFromFileType(ast, id, typeId);

		if (!name.IsNone() && !ast.Has<CNamespace>(id))
		{
			ast.Add<CNamespace>(id, name);
		}
		return id;
	}

	void RemoveTypes(TAccessRef<TWrite<CChild>, TWrite<CParent>, CFileRef> access,
	    TView<Id> typeIds, bool removeFromDisk)
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
		p::RemoveId(access, typeIds, true);
	}

	void SerializeType(Tree& ast, Id id, String& data)
	{
		if (!Ensure(ast.Has<CDeclType>(id)))
		{
			return;
		}

		JsonFormatWriter writer{};
		p::EntityWriter w{writer.GetWriter(), ast};
		w.BeginObject();

		w.Next("type", ast.Get<CDeclType>(id).typeId);
		w.SerializeEntity(id, gTypeComponents);

		data = writer.ToString();
	}

	void DeserializeType(Tree& ast, Id id, const String& data)
	{
		JsonFormatReader reader{data};
		if (!reader.IsValid())
		{
			return;
		}

		p::EntityReader r{reader, ast};
		r.BeginObject();

		p::Tag typeId;
		r.Next("type", typeId);
		InitTypeFromFileType(ast, id, typeId);

		r.SerializeEntity(id, gTypeComponents);
	}


	Id FindTypeByPath(Tree& ast, p::StringView path)
	{
		if (auto* types = ast.TryGetStatic<STypes>())
		{
			// TODO: Replace once StringView TMap is more stable
			if (Id* id = types->typesByPath.Find(p::Tag{path}))
			{
				return *id;
			}
		}
		return NoId;
	}

	bool IsClassType(TAccessRef<CDeclClass> access, Id typeId)
	{
		return access.Has<CDeclClass>(typeId);
	}

	bool IsStructType(TAccessRef<CDeclStruct> access, Id typeId)
	{
		return access.Has<CDeclStruct>(typeId);
	}

	bool IsStaticType(TAccessRef<CDeclStatic> access, Id typeId)
	{
		return access.Has<CDeclStatic>(typeId);
	}

	bool HasVariables(TAccessRef<CDeclType> access, Id typeId)
	{
		if (const RiftType* fileType = FindFileType(access, typeId))
		{
			return fileType->settings.hasVariables;
		}
		return false;
	}

	bool HasFunctions(TAccessRef<CDeclType> access, Id typeId)
	{
		if (const RiftType* fileType = FindFileType(access, typeId))
		{
			return fileType->settings.hasFunctions;
		}
		return false;
	}

	bool HasFunctionBodies(TAccessRef<CDeclType> access, Id typeId)
	{
		if (const RiftType* fileType = FindFileType(access, typeId))
		{
			return fileType->settings.hasFunctions && fileType->settings.hasFunctionBodies;
		}
		return false;
	}


	Id AddVariable(TypeRef type, Tag name)
	{
		Tree& ast = type.GetContext();

		Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CDeclVariable, CParent>(id);

		if (type)
		{
			p::AttachId(ast, type, id);
		}
		return id;
	}

	Id AddFunction(TypeRef type, Tag name)
	{
		Tree& ast = type.GetContext();

		Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CDeclFunction, CParent>(id);
		ast.Add<CStmtOutput>(id);

		if (type)
		{
			p::AttachId(ast, type, id);
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
			p::AttachId(ast, type.GetId(), id);
		}
		return id;
	}

	Id AddFunctionInput(Tree& ast, Id functionId, Tag name)
	{
		Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CExprTypeId>(id);
		ast.Add<CExprType>(id);
		p::AttachId(ast, functionId, id);
		ast.GetOrAdd<CExprOutputs>(functionId).Add(id);
		return id;
	}

	Id AddFunctionOutput(Tree& ast, Id functionId, Tag name)
	{
		Id id = ast.Create();
		ast.Add<CNamespace>(id, name);
		ast.Add<CExprTypeId>(id);
		ast.Add<CExprType>(id);
		p::AttachId(ast, functionId, id);
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
		p::AttachId(ast, id, valueId);
		ast.Add<CExprInputs>(id).Add(valueId);

		TArray<Id> outIds(2);
		ast.Create(outIds);
		p::AttachId(ast, id, outIds);
		ast.Add<CStmtOutputs>(id, Move(outIds));

		if (type)
		{
			p::AttachId(ast, type.GetId(), id);
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
			p::AttachId(ast, type.GetId(), returnId);
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
			p::AttachId(ast, type.GetId(), id);
		}
		return id;
	}

	Id AddDeclarationReference(TypeRef type, Id declId)
	{
		Tree& ast   = type.GetContext();
		const Id id = ast.Create();

		ast.Add<CExprDeclRef>(id);
		ast.Add<CExprOutputs>(id).Add(id);    // Types gets resolved by a system later

		const Id typeId = p::GetIdParent(ast, declId);
		Check(!IsNone(typeId));
		auto& declRefExpr           = ast.Add<CExprDeclRef>(id);
		declRefExpr.ownerName       = ast.Get<CNamespace>(typeId).name;
		declRefExpr.name            = ast.Get<CNamespace>(declId).name;
		auto& declRefExprId         = ast.Add<CExprDeclRefId>(id);
		declRefExprId.declarationId = declId;

		if (type)
		{
			p::AttachId(ast, type.GetId(), id);
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
			p::AttachId(ast, type.GetId(), id);
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
		p::AttachId(ast, id, inputs.pinIds);
		if (type)
		{
			p::AttachId(ast, type.GetId(), id);
		}
		return id;
	}

	Id FindChildByName(TAccessRef<CNamespace, CParent> access, Id ownerId, Tag functionName)
	{
		if (!IsNone(ownerId))
		{
			TArray<Id> children;
			p::GetIdChildren(access, ownerId, children);
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

	void RemoveNodes(const RemoveAccess& access, TView<Id> ids)
	{
		ScopedChange(access, ids);
		p::RemoveId(access, ids, true);
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


	void RegisterFileType(RiftType&& type)
	{
		gFileTypes.AddUniqueSorted(Move(type));
	}
	void UnregisterFileType(p::Tag typeId)
	{
		gFileTypes.RemoveSorted(typeId);
	}

	p::TView<const RiftType> GetFileTypes()
	{
		return gFileTypes;
	}

	const RiftType* FindFileType(p::Tag typeId)
	{
		const i32 index = gFileTypes.FindSortedEqual(typeId);
		return index != NO_INDEX ? gFileTypes.Data() + index : nullptr;
	}

	const RiftType* FindFileType(p::TAccessRef<ast::CDeclType> access, ast::Id typeId)
	{
		if (const auto* type = access.TryGet<const ast::CDeclType>(typeId))
		{
			return FindFileType(type->typeId);
		}
		return nullptr;
	}
}    // namespace rift::ast
