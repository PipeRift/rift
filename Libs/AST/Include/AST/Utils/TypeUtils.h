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



namespace rift::AST
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

	Id CreateType(Tree& ast, p::Tag typeId, Tag name = Tag::None(), StringView path = {});

	void RemoveTypes(TAccessRef<TWrite<CChild>, TWrite<CParent>, CFileRef> access, TView<Id> types,
	    bool removeFromDisk = false);

	void SerializeType(Tree& ast, Id id, String& data);
	void DeserializeType(Tree& ast, Id id, const String& data);

	Id FindTypeByPath(Tree& ast, p::StringView path);
	bool IsClassType(const Tree& ast, Id typeId);
	bool IsStructType(const Tree& ast, Id typeId);
	bool IsStaticType(const Tree& ast, Id typeId);
	bool HasVariables(TAccess<CDeclType> access, Id typeId);
	bool HasFunctions(TAccess<CDeclType> access, Id typeId);
	bool HasFunctionBodies(TAccess<CDeclType> access, Id typeId);

	Id AddVariable(TypeRef type, Tag name);
	Id AddFunction(TypeRef type, Tag name);

	Id AddCall(TypeRef type, Id targetFunctionId);
	Id AddFunctionInput(Tree& ast, Id functionId, Tag name = Tag::None());
	Id AddFunctionOutput(Tree& ast, Id functionId, Tag name = Tag::None());

	Id AddIf(TypeRef type);
	Id AddReturn(TypeRef type);

	Id AddLiteral(TypeRef type, Id literalTypeId);
	Id AddDeclarationReference(TypeRef type, Id declId);
	Id AddUnaryOperator(TypeRef type, UnaryOperatorType operatorType);
	Id AddBinaryOperator(TypeRef type, BinaryOperatorType operatorType);

	Id FindChildByName(TAccessRef<CNamespace, CParent> access, Id ownerId, Tag functionName);

	using RemoveAccess = TAccess<TWrite<CChanged>, TWrite<CFileDirty>, TWrite<CStmtInput>,
	    TWrite<CStmtOutputs>, TWrite<CParent>, TWrite<CChild>, CFileRef>;
	void RemoveNodes(const RemoveAccess& access, TView<Id> ids);

	bool CopyExpressionType(TAccessRef<TWrite<CExprTypeId>> access, Id sourcePinId, Id targetPinId);


	void RegisterFileType(RiftType&& descriptor);
	void UnregisterFileType(p::Tag typeId);

	p::TView<const RiftType> GetFileTypes();
	const RiftType* FindFileType(p::Tag typeId);
	const RiftType* FindFileType(p::TAccessRef<AST::CDeclType> access, AST::Id typeId);

	template<typename TagType>
	void RegisterFileType(p::Tag typeId, RiftTypeSettings settings)
	{
		RegisterFileType(
		    {.id = typeId, .tagType = TagType::GetStaticType(), .settings = p::Move(settings)});
	}
}    // namespace rift::AST
