// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CNamespace.h>
#include <Pipe/Core/Platform.h>
#include <Pipe/Core/StringView.h>
#include <Pipe/Memory/BigBestFitArena.h>
#include <Pipe/Memory/Block.h>
#include <PipeECS.h>
#include <PipeVectors.h>
#include <UI/UI.h>


namespace rift::Editor
{
	struct InspectorPanel
	{
		AST::Id id        = AST::NoId;
		bool pendingFocus = true;
		bool open         = true;

		bool operator==(const InspectorPanel& other) const
		{
			return id == other.id;
		}
	};

	struct ASTDebugger
	{
		bool open          = false;
		bool showHierarchy = true;

		AST::Id selectedNode = AST::NoId;
		ImGuiTextFilter filter;

		// First inspector is the main inspector
		InspectorPanel mainInspector;
		p::TArray<InspectorPanel> secondaryInspectors;


		ASTDebugger();

		void Draw(AST::Tree& ast);

	private:
		using DrawNodeAccess =
		    p::TAccessRef<AST::CNamespace, AST::CFileRef, AST::CParent, AST::CChild, AST::CModule>;
		void DrawNode(DrawNodeAccess access, AST::Id nodeId, bool showChildren);

		void OnInspectEntity(AST::Id id);

		void DrawEntityInspector(p::StringView label, p::StringView id, AST::Tree& ast,
		    InspectorPanel& inspector, bool* open = nullptr);

		void OpenAvailableSecondaryInspector(AST::Id id);
	};
}    // namespace rift::Editor
