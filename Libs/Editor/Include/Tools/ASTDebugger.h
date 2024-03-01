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


namespace rift::editor
{
	struct InspectorPanel
	{
		ast::Id id        = ast::NoId;
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

		ast::Id selectedNode = ast::NoId;
		ImGuiTextFilter filter;

		// First inspector is the main inspector
		InspectorPanel mainInspector;
		p::TArray<InspectorPanel> secondaryInspectors;


		ASTDebugger();

		void Draw(ast::Tree& ast);

	private:
		using DrawNodeAccess =
		    p::TAccessRef<ast::CNamespace, ast::CFileRef, ast::CParent, ast::CChild, ast::CModule>;
		void DrawNode(DrawNodeAccess access, ast::Id nodeId, bool showChildren);

		void OnInspectEntity(ast::Id id);

		void DrawEntityInspector(p::StringView label, p::StringView id, ast::Tree& ast,
		    InspectorPanel& inspector, bool* open = nullptr);

		void OpenAvailableSecondaryInspector(ast::Id id);
	};
}    // namespace rift::editor
