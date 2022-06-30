// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclFunctionLibrary.h>
#include <AST/Components/CDeclNative.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprType.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CParent.h>
#include <AST/Components/CType.h>
#include <AST/Tree.h>
#include <AST/Utils/TransactionUtils.h>
#include <Pipe/Core/StringView.h>
#include <Pipe/ECS/Filtering.h>


namespace rift
{
	struct CTypeEditor;


	enum class DrawFieldFlags
	{
		None      = 0,
		HideValue = 1 << 0
	};

	using TVariableAccessRef = TAccessRef<TWrite<CDeclVariable>, TWrite<CIdentifier>, CType,
	    CDeclNative, CDeclStruct, CDeclClass, CParent>;

	void DrawField(AST::Tree& ast, CTypeEditor& editor, AST::Id functionId, AST::Id fieldId,
	    DrawFieldFlags flags = DrawFieldFlags::None);

	void DrawVariable(TVariableAccessRef access, CTypeEditor& editor, AST::Id variableId);
	void DrawFunction(AST::Tree& ast, CTypeEditor& editor, AST::Id functionId);

	void DrawElementsPanel(AST::Tree& ast, AST::Id typeId);
}    // namespace rift
