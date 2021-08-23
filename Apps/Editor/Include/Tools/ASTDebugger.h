// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Math/Vector.h>
#include <Memory/Arenas/BigBestFitArena.h>
#include <Memory/Blocks/Block.h>
#include <Platform/Platform.h>
#include <Strings/StringView.h>


namespace Rift
{
	struct ASTDebugger
	{
		bool open = false;

		AST::Id selectedEntity = AST::NoId;


		ASTDebugger();

		void Draw(AST::Tree& ast);

	private:
		static void DrawEntity(AST::Tree& ast, AST::Id entity);
	};
}    // namespace Rift
