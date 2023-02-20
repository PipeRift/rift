// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CDeclType.h"
#include "AST/Components/CExprBinaryOperator.h"
#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprType.h"
#include "AST/Components/CExprUnaryOperator.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Tree.h"
#include "AST/TypeRef.h"

#include <Pipe/ECS/Access.h>
#include <Pipe/ECS/Components/CChild.h>
#include <Pipe/ECS/Components/CParent.h>


namespace rift::AST
{
	void InitTypeFromFileType(Tree& ast, Id id, p::Tag typeId);

	Id CreateType(Tree& ast, p::Tag typeId, Tag name = Tag::None(), StringView path = {});

	void RemoveTypes(TAccessRef<TWrite<CChild>, TWrite<CParent>, CFileRef> access, TSpan<Id> types,
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
	void RemoveNodes(const RemoveAccess& access, TSpan<Id> ids);

	bool CopyExpressionType(TAccessRef<TWrite<CExprTypeId>> access, Id sourcePinId, Id targetPinId);
}    // namespace rift::AST
