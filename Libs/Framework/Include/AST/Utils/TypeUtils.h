// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "AST/Components/CChild.h"
#include "AST/Components/CDeclFunction.h"
#include "AST/Components/CExprBinaryOperator.h"
#include "AST/Components/CExprInputs.h"
#include "AST/Components/CExprOutputs.h"
#include "AST/Components/CExprUnaryOperator.h"
#include "AST/Components/CFileRef.h"
#include "AST/Components/CIdentifier.h"
#include "AST/Components/CParent.h"
#include "AST/Components/CStmtInput.h"
#include "AST/Components/CStmtOutputs.h"
#include "AST/Components/CType.h"
#include "AST/Components/Tags/CChanged.h"
#include "AST/Components/Tags/CDirty.h"
#include "AST/Tree.h"
#include "AST/TypeRef.h"

#include <ECS/Access.h>


namespace Rift::Types
{
	void InitTypeFromCategory(AST::Tree& ast, AST::Id id, Type category);
	Type GetCategory(AST::Tree& ast, AST::Id id);

	AST::Id CreateType(
	    AST::Tree& ast, Type type, Name name = Name::None(), const Pipe::Path& path = {});

	void RemoveTypes(TAccessRef<TWrite<CChild>, TWrite<CParent>, CFileRef> access,
	    TSpan<AST::Id> types, bool removeFromDisk = false);

	void Serialize(AST::Tree& ast, AST::Id id, String& data);
	void Deserialize(AST::Tree& ast, AST::Id id, const String& data);

	AST::Id FindTypeByPath(AST::Tree& ast, const Pipe::Path& path);
	bool IsClass(const AST::Tree& ast, AST::Id typeId);
	bool IsStruct(const AST::Tree& ast, AST::Id typeId);
	bool IsFunctionLibrary(const AST::Tree& ast, AST::Id typeId);
	bool CanContainVariables(const AST::Tree& ast, AST::Id typeId);
	bool CanContainFunctions(const AST::Tree& ast, AST::Id typeId);


	AST::Id AddVariable(AST::TypeRef type, Name name);
	AST::Id AddFunction(AST::TypeRef type, Name name);

	AST::Id AddCall(AST::TypeRef type, AST::Id targetFunctionId);
	AST::Id AddFunctionInput(AST::Tree& ast, AST::Id functionId, Name name = Name::None());
	AST::Id AddFunctionOutput(AST::Tree& ast, AST::Id functionId, Name name = Name::None());

	AST::Id AddIf(AST::TypeRef type);
	AST::Id AddReturn(AST::TypeRef type);

	AST::Id AddLiteral(AST::TypeRef type, AST::Id literalTypeId);
	AST::Id AddDeclarationReference(AST::TypeRef type, AST::Id declId);
	AST::Id AddUnaryOperator(AST::TypeRef type, UnaryOperatorType operatorType);
	AST::Id AddBinaryOperator(AST::TypeRef type, BinaryOperatorType operatorType);

	AST::Id FindFunctionByName(
	    TAccessRef<CDeclFunction, CIdentifier, CParent> access, Name ownerName, Name functionName);

	using RemoveAccess = TAccess<TWrite<CChanged>, TWrite<CFileDirty>, TWrite<CStmtInput>,
	    TWrite<CStmtOutputs>, TWrite<CParent>, TWrite<CChild>, CFileRef>;
	void RemoveNodes(const RemoveAccess& access, TSpan<AST::Id> ids);

}    // namespace Rift::Types
