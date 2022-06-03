// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <AST/Components/CChild.h>
#include <AST/Components/CFileRef.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CParent.h>
#include <ECS/Filtering.h>
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
		using DrawNodeAccess = TAccessRef<const CIdentifier, const CFileRef, const CParent>;
		void DrawNode(DrawNodeAccess access, AST::Id nodeId, bool showChildren);
	};
}    // namespace Rift
