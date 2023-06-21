// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <AST/Components/CDeclClass.h>
#include <AST/Components/CDeclFunction.h>
#include <AST/Components/CDeclNative.h>
#include <AST/Components/CDeclStatic.h>
#include <AST/Components/CDeclStruct.h>
#include <AST/Components/CDeclType.h>
#include <AST/Components/CDeclVariable.h>
#include <AST/Components/CExprType.h>
#include <AST/Components/CNamespace.h>
#include <AST/Tree.h>
#include <AST/Utils/TransactionUtils.h>
#include <Pipe/Core/StringView.h>
#include <Pipe/ECS/Components/CParent.h>
#include <Pipe/PipeECS.h>


namespace rift::Editor
{
	struct CTypeEditor;


	enum class DrawFieldFlags
	{
		None      = 0,
		HideValue = 1 << 0
	};

	using TVariableAccessRef =
	    p::TAccessRef<p::TWrite<AST::CDeclVariable>, p::TWrite<AST::CNamespace>, AST::CDeclType,
	        AST::CDeclNative, AST::CDeclStruct, AST::CDeclClass, AST::CParent>;

	void DrawField(AST::Tree& ast, CTypeEditor& editor, AST::Id functionId, AST::Id fieldId,
	    DrawFieldFlags flags = DrawFieldFlags::None);

	void DrawVariable(TVariableAccessRef access, CTypeEditor& editor, AST::Id variableId);
	void DrawFunction(AST::Tree& ast, CTypeEditor& editor, AST::Id functionId);

	void DrawElementsPanel(AST::Tree& ast, AST::Id typeId);
}    // namespace rift::Editor
