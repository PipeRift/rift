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
	void DrawDetailsPanel(AST::Tree& ast, AST::Id typeId);
}    // namespace rift
