// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <AST/Components/CChild.h>
#include <AST/Components/CFileRef.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CNamespace.h>
#include <AST/Components/CParent.h>
#include <Pipe/Core/Platform.h>
#include <Pipe/Core/StringView.h>
#include <Pipe/ECS/Filtering.h>
#include <Pipe/Math/Vector.h>
#include <Pipe/Memory/Arenas/BigBestFitArena.h>
#include <Pipe/Memory/Blocks/Block.h>
#include <UI/UI.h>


namespace rift
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
		using DrawNodeAccess = TAccessRef<CNamespace, CFileRef, CParent, CChild, CModule>;
		void DrawNode(DrawNodeAccess access, AST::Id nodeId, bool showChildren);
	};
}    // namespace rift
