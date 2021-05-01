// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Lang/AST.h"

#include <Math/Vector.h>
#include <Memory/Arenas/BigBestFitArena.h>
#include <Memory/Blocks/Block.h>
#include <Platform/Platform.h>
#include <Strings/StringView.h>


namespace Rift
{
	struct ASTDebugger
	{
		bool open = true;

		AST::Id selectedEntity{};


		ASTDebugger();

		void Draw(AST::AbstractSyntaxTree& ast);

	private:
		void DrawEntity(AST::AbstractSyntaxTree& ast, AST::Id entity);
	};
}    // namespace Rift
