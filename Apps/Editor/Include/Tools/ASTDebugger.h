// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <Math/Vector.h>
#include <Memory/Arenas/BigBestFitArena.h>
#include <Memory/Blocks/Block.h>
#include <Platform/Platform.h>
#include <Strings/StringView.h>
#include <UI/UI.h>


namespace Rift
{
	struct ASTDebugger
	{
		bool open          = false;
		bool showHierarchy = true;

		AST::Id selectedNode = AST::NoId;
		ImGuiTextFilter filter;


		ASTDebugger();

		void Draw(AST::Tree& ast);

	private:
		void DrawNode(AST::Tree& ast, AST::Id nodeId, bool showChildren);
	};
}    // namespace Rift
