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
#include <Pipe/PipeECS.h>


namespace rift::Editor
{
	void DrawDetailsPanel(AST::Tree& ast, AST::Id typeId);
}    // namespace rift::Editor
