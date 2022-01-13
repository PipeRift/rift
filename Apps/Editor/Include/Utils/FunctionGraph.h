// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Components/CTypeEditor.h"

#include <AST/Tree.h>
#include <Math/Vector.h>


namespace Rift
{
	struct DockSpaceLayout;
}

namespace Rift::Graph
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
	};
	inline Settings settings{};


	void Init();
	void Shutdown();


	void PushNodeStyle();
	void PopNodeStyle();
	void PushInnerNodeStyle();
	void PopInnerNodeStyle();

	void DrawContextMenu(AST::Tree& ast);
	void DrawTypeGraph(AST::Tree& ast, AST::Id typeId, CTypeEditor& typeEditor);

	void DrawFunctionDecl(AST::Tree& ast, AST::Id functionId);
	void DrawCallNode(AST::Tree& ast, AST::Id id, StringView name, StringView typeName);
	void DrawBoolLiteralNode(AST::Id id, bool& value);
	void DrawStringLiteralNode(AST::Id id, String& value);

	void SetNodePosition(AST::Id id, v2 position);
	v2 GetNodePosition(AST::Id id);

	void SnapNodeDimensionsToGrid();
}    // namespace Rift::Graph
