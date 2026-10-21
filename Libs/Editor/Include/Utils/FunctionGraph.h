// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Components/CTypeEditor.h"
#include "Utils/Nodes.h"

#include <AST/Tree.h>
#include <PipeMath.h>


namespace rift
{
	struct DockSpaceLayout;
}

namespace rift::editor::Graph
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

		float GetSpaceHeight() const;
		v2 GetGridPosition(v2 screenPosition) const;
	};
	inline Settings settings{};


	void DrawLiteralBool(ast::Tree& ast, ast::Id id, bool& value);
	void DrawLiteralString(ast::Tree& ast, ast::Id id, String& value);

	void Init();
	void Shutdown();

	// Applies the grid size and zoom configuration from the user settings to the node editor.
	void ApplyGraphSettings();

	void PushNodeStyle();
	void PopNodeStyle();
	void PushInnerNodeStyle();
	void PopInnerNodeStyle();

	void DrawTypeGraph(ast::Tree& ast, ast::Id typeId, CTypeEditor& typeEditor);

	void SetNodePosition(ast::Id id, v2 position);
	v2 GetNodePosition(ast::Id id);
}    // namespace rift::editor::Graph
