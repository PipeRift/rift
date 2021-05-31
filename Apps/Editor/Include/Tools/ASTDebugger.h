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

		void Draw(AST::Tree& ast);

	private:
		static void DrawEntity(AST::Tree& ast, AST::Id entity);
	};
}    // namespace Rift
