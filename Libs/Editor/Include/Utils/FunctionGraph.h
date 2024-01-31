// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Components/CTypeEditor.h"

#include <AST/Tree.h>
#include <PipeVectors.h>


namespace rift
{
	struct DockSpaceLayout;
}

namespace rift::Editor::Graph
{
	struct Settings
	{
	public:
		float verticalMargin  = 1.f;
		float verticalPadding = 0.f;

	protected:
		float gridSize    = 16.f;
		float invGridSize = 1.f / gridSize;

	public:
		void SetGridSize(float size);
		float GetGridSize() const;
		float GetInvGridSize() const;

		float GetSpaceHeight(u32 height) const;
		v2 GetContentPadding() const;
		v2 GetGridPosition(v2 screenPosition) const;
	};
	inline Settings settings{};


	void DrawLiteralBool(ast::Tree& ast, ast::Id id, bool& value);
	void DrawLiteralInt(ast::Tree& ast, ast::Id id, String& value);
	void DrawLiteralString(ast::Tree& ast, ast::Id id, String& value);

	void DrawFunctionDecl(ast::Tree& ast, ast::Id functionId);
	void DrawCallNode(ast::Tree& ast, ast::Id id, StringView name, StringView ownerName);

	void Init();
	void Shutdown();

	void PushNodeStyle();
	void PopNodeStyle();
	void PushInnerNodeStyle();
	void PopInnerNodeStyle();

	void DrawTypeGraph(ast::Tree& ast, ast::Id typeId, CTypeEditor& typeEditor);

	void SetNodePosition(ast::Id id, v2 position);
	v2 GetNodePosition(ast::Id id);

	void SnapNodeDimensionsToGrid();
}    // namespace rift::Editor::Graph
