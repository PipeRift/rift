// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <AST/Components/CNamespace.h>
#include <AST/Components/Declarations.h>
#include <AST/Components/Expressions.h>
#include <AST/Tree.h>
#include <AST/Utils/TransactionUtils.h>
#include <Pipe/Core/StringView.h>
#include <PipeECS.h>


namespace rift::Editor
{
	void DrawDetailsPanel(AST::Tree& ast, AST::Id typeId);
}    // namespace rift::Editor
