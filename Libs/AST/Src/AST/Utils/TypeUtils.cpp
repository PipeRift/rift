// Copyright 2015-2026 Piperift. All Rights Reserved.

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
			ast.AddByTypeId(fileType->tagType, id);
		}
	}

	Id CreateType(Tree& ast, p::Tag typeId, Tag name, StringView path)
	{
		Id id = p::AddId(ast);
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

	void RemoveTypes(p::TIdScopeRef<Writes<CChild, CParent>, CFileRef> scope, TView<Id> typeIds,
	    bool removeFromDisk)
	{
		if (removeFromDisk)
		{
			for (Id id : typeIds)
			{
				if (const auto* file = scope.TryGet<const CFileRef>(id))
				{
					Delete(file->path, true, false);
				}
			}
		}
		p::RmId(scope.GetContext(), typeIds);
	}

	void SerializeType(Tree& ast, Id id, String& data)
	{
		if (!P_Ensure(ast.Has<CDeclType>(id)))
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

	bool IsClassType(p::TIdScopeRef<CDeclClass> scope, Id typeId)
	{
		return scope.Has<CDeclClass>(typeId);
	}

	bool IsStructType(p::TIdScopeRef<CDeclStruct> scope, Id typeId)
	{
		return scope.Has<CDeclStruct>(typeId);
	}

	bool IsStaticType(p::TIdScopeRef<CDeclStatic> scope, Id typeId)
	{
		return scope.Has<CDeclStatic>(typeId);
	}

	bool HasVariables(p::TIdScopeRef<CDeclType> scope, Id typeId)
	{
		if (const RiftType* fileType = FindFileType(scope, typeId))
		{
			return fileType->settings.hasVariables;
		}
		return false;
	}

	bool HasFunctions(p::TIdScopeRef<CDeclType> scope, Id typeId)
	{
		if (const RiftType* fileType = FindFileType(scope, typeId))
		{
			return fileType->settings.hasFunctions;
		}
		return false;
	}

	bool HasFunctionBodies(p::TIdScopeRef<CDeclType> scope, Id typeId)
	{
		if (const RiftType* fileType = FindFileType(scope, typeId))
		{
			return fileType->settings.hasFunctions && fileType->settings.hasFunctionBodies;
		}
		return false;
	}


	Id AddVariable(TypeRef type, Tag name)
	{
		Tree& ast = type.GetContext();

		Id id = p::AddId(ast);
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

		Id id = p::AddId(ast);
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
		const Id id = p::AddId(ast);

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
		Id id = p::AddId(ast);
		ast.Add<CNamespace>(id, name);
		ast.Add<CExprTypeId>(id);
		ast.Add<CExprType>(id);
		p::AttachId(ast, functionId, id);
		ast.GetOrAdd<CExprOutputs>(functionId).Add(id);
		return id;
	}

	Id AddFunctionOutput(Tree& ast, Id functionId, Tag name)
	{
		Id id = p::AddId(ast);
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
		const Id id = p::AddId(ast);
		ast.Add<CStmtIf>(id);
		ast.Add<CStmtInput>(id);

		// Bool input
		const Id valueId = p::AddId(ast);
		ast.Add<CExprTypeId>(valueId, {.id = ast.GetNativeTypes().boolId});
		ast.Add<CExprType>(id).type = GetNamespace(ast, ast.GetNativeTypes().boolId);
		p::AttachId(ast, id, valueId);
		ast.Add<CExprInputs>(id).Add(valueId);

		TArray<Id> outIds(2);
		p::AddId(ast, outIds);
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
		const Id returnId = p::AddId(ast);
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
		const Id id = p::AddId(ast);
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
			p::RmId(ast, id);
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
		const Id id = p::AddId(ast);

		ast.Add<CExprDeclRef>(id);
		ast.Add<CExprOutputs>(id).Add(id);    // Types gets resolved by a system later

		const Id typeId = p::GetIdParent(ast, declId);
		P_Check(!IsNone(typeId));
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
		const Id id = p::AddId(ast);
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
		const Id id = p::AddId(ast);
		ast.Add<CExprBinaryOperator>(id, {operatorType});
		ast.Add<CExprOutputs>(id).Add(id);

		auto& inputs = ast.Add<CExprInputs>(id);
		inputs.Resize(2);
		p::AddId(ast, inputs.pinIds);
		p::AttachId(ast, id, inputs.pinIds);
		if (type)
		{
			p::AttachId(ast, type.GetId(), id);
		}
		return id;
	}

	Id FindChildByName(p::TIdScopeRef<CNamespace, CParent> scope, Id ownerId, Tag functionName)
	{
		if (!IsNone(ownerId))
		{
			TArray<Id> children;
			p::GetIdChildren(scope, ownerId, children);
			for (Id childId : children)
			{
				const auto* ns = scope.TryGet<const CNamespace>(childId);
				if (ns && ns->name == functionName)
				{
					return childId;
				}
			}
		}
		return NoId;
	}

	void RemoveNodes(const RemoveScope& scope, TView<Id> ids)
	{
		ScopedChange(scope, ids);
		p::RmId(scope.GetContext(), ids);
	}

	bool CopyExpressionType(
	    p::TIdScopeRef<Writes<CExprTypeId>> scope, Id sourcePinId, Id targetPinId)
	{
		auto* sourceType = scope.TryGet<const CExprTypeId>(sourcePinId);
		auto* targetType = scope.TryGet<CExprTypeId>(targetPinId);
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
			scope.Add<CExprTypeId>(targetPinId, *sourceType);
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
		const i32 index = gFileTypes.FindSorted(typeId);
		return index != NO_INDEX ? gFileTypes.Data() + index : nullptr;
	}

	const RiftType* FindFileType(p::TIdScopeRef<ast::CDeclType> scope, ast::Id typeId)
	{
		if (const auto* type = scope.TryGet<const ast::CDeclType>(typeId))
		{
			return FindFileType(type->typeId);
		}
		return nullptr;
	}
}    // namespace rift::ast
