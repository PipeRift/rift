// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CExprBinaryOperator.h"
#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprType.h"
#include "AST/Components/CExprUnaryOperator.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Components/CType.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Tree.h"
#include "AST/TypeRef.h"

#include <Pipe/ECS/Access.h>
#include <Pipe/ECS/Components/CChild.h>
#include <Pipe/ECS/Components/CParent.h>


namespace rift::AST
{
	void InitTypeFromCategory(Tree& ast, Id id, RiftType category);
	RiftType GetTypeCategory(Tree& ast, Id id);

	Id CreateType(Tree& ast, RiftType type, Name name = Name::None(), StringView path = {});

	void RemoveTypes(TAccessRef<TWrite<CChild>, TWrite<CParent>, CFileRef> access, TSpan<Id> types,
	    bool removeFromDisk = false);

	void SerializeType(Tree& ast, Id id, String& data);
	void DeserializeType(Tree& ast, Id id, const String& data);

	Id FindTypeByPath(Tree& ast, p::StringView path);
	bool IsClassType(const Tree& ast, Id typeId);
	bool IsStructType(const Tree& ast, Id typeId);
	bool IsStaticType(const Tree& ast, Id typeId);
	bool CanContainVariables(const Tree& ast, Id typeId);
	bool CanContainFunctions(const Tree& ast, Id typeId);
	bool CanEditFunctionBodies(const Tree& ast, Id typeId);

	Id AddVariable(TypeRef type, Name name);
	Id AddFunction(TypeRef type, Name name);

	Id AddCall(TypeRef type, Id targetFunctionId);
	Id AddFunctionInput(Tree& ast, Id functionId, Name name = Name::None());
	Id AddFunctionOutput(Tree& ast, Id functionId, Name name = Name::None());

	Id AddIf(TypeRef type);
	Id AddReturn(TypeRef type);

	Id AddLiteral(TypeRef type, Id literalTypeId);
	Id AddDeclarationReference(TypeRef type, Id declId);
	Id AddUnaryOperator(TypeRef type, UnaryOperatorType operatorType);
	Id AddBinaryOperator(TypeRef type, BinaryOperatorType operatorType);

	Id FindChildByName(TAccessRef<CNamespace, CParent> access, Id ownerId, Name functionName);

	using RemoveAccess = TAccess<TWrite<CChanged>, TWrite<CFileDirty>, TWrite<CStmtInput>,
	    TWrite<CStmtOutputs>, TWrite<CParent>, TWrite<CChild>, CFileRef>;
	void RemoveNodes(const RemoveAccess& access, TSpan<Id> ids);

	bool CopyExpressionType(TAccessRef<TWrite<CExprTypeId>> access, Id sourcePinId, Id targetPinId);
}    // namespace rift::AST
