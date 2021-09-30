// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "AST/Tree.h"


namespace Rift::Graph
{
	struct Settings
	{
	protected:
		float gridSize    = 12.f;
		float invGridSize = 1 / gridSize;

	public:
		void SetGridSize(float size);
		float GetGridSize() const;
		float GetInvGridSize() const;
	};
	inline Settings settings{};


	void Init();
	void Shutdown();


	void PushNodeStyle();
	void PopNodeStyle();

	void DrawFunctionGraph(AST::Tree& ast, AST::Id typeId, struct DockSpaceLayout& layout);
	void DrawFunctionNodes(AST::Tree& ast, AST::Id functionId);


	void DrawBoolLiteralNode(AST::Id id);

	void SetNodePosition(AST::Id id, v2 position);
	v2 GetNodePosition(AST::Id id);
}
