// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/Declarations.h"
#include "AST/Components/Expressions.h"
#include "AST/Components/Statements.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Tree.h"
#include "AST/TypeRef.h"

#include <PipeECS.h>


namespace rift::ast
{
	struct RiftTypeSettings
	{
		p::String displayName;
		p::String category;
		bool hasVariables      = false;
		bool hasFunctions      = false;
		bool hasFunctionBodies = true;
	};

	struct RiftType
	{
		p::Tag id;
		p::StructType* tagType = nullptr;
		RiftTypeSettings settings;

		bool operator<(const RiftType& other) const
		{
			return id < other.id;
		}
		friend bool operator<(const p::Tag& lhs, const RiftType& rhs)
		{
			return lhs < rhs.id;
		}
		friend bool operator<(const RiftType& lhs, const p::Tag& rhs)
		{
			return lhs.id < rhs;
		}
	};


	void InitTypeFromFileType(Tree& ast, Id id, p::Tag typeId);

	Id CreateType(Tree& ast, p::Tag typeId, p::Tag name = p::Tag::None(), p::StringView path = {});

	void RemoveTypes(p::TAccessRef<p::TWrite<CChild>, p::TWrite<CParent>, CFileRef> access,
	    p::TView<Id> types, bool removeFromDisk = false);

	void SerializeType(Tree& ast, p::Id id, p::String& data);
	void DeserializeType(Tree& ast, p::Id id, const p::String& data);

	Id FindTypeByPath(Tree& ast, p::StringView path);
	bool IsClassType(p::TAccessRef<CDeclClass> access, Id typeId);
	bool IsStructType(p::TAccessRef<CDeclStruct> access, Id typeId);
	bool IsStaticType(p::TAccessRef<CDeclStatic> access, Id typeId);
	bool HasVariables(p::TAccessRef<CDeclType> access, Id typeId);
	bool HasFunctions(p::TAccessRef<CDeclType> access, Id typeId);
	bool HasFunctionBodies(p::TAccessRef<CDeclType> access, Id typeId);

	Id AddVariable(TypeRef type, p::Tag name);
	Id AddFunction(TypeRef type, p::Tag name);

	Id AddCall(TypeRef type, Id targetFunctionId);
	Id AddFunctionInput(Tree& ast, Id functionId, p::Tag name = p::Tag::None());
	Id AddFunctionOutput(Tree& ast, Id functionId, p::Tag name = p::Tag::None());

	Id AddIf(TypeRef type);
	Id AddReturn(TypeRef type);

	Id AddLiteral(TypeRef type, Id literalTypeId);
	Id AddDeclarationReference(TypeRef type, Id declId);
	Id AddUnaryOperator(TypeRef type, UnaryOperatorType operatorType);
	Id AddBinaryOperator(TypeRef type, BinaryOperatorType operatorType);

	Id FindChildByName(p::TAccessRef<CNamespace, CParent> access, Id ownerId, p::Tag functionName);

	using RemoveAccess =
	    p::TAccess<p::TWrite<CChanged>, p::TWrite<CFileDirty>, p::TWrite<CStmtInput>,
	        p::TWrite<CStmtOutputs>, p::TWrite<CParent>, p::TWrite<CChild>, CFileRef>;
	void RemoveNodes(const RemoveAccess& access, p::TView<Id> ids);

	bool CopyExpressionType(
	    p::TAccessRef<p::TWrite<CExprTypeId>> access, Id sourcePinId, Id targetPinId);


	void RegisterFileType(RiftType&& descriptor);
	void UnregisterFileType(p::Tag typeId);

	p::TView<const RiftType> GetFileTypes();
	const RiftType* FindFileType(p::Tag typeId);
	const RiftType* FindFileType(p::TAccessRef<CDeclType> access, ast::Id typeId);

	template<typename TagType>
	void RegisterFileType(p::Tag typeId, RiftTypeSettings settings)
	{
		RegisterFileType(
		    {.id = typeId, .tagType = TagType::GetStaticType(), .settings = p::Move(settings)});
	}
}    // namespace rift::ast
