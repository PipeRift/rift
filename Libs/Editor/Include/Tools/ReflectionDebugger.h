// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <UI/UI.h>


namespace rift::editor
{
	using namespace p;


	struct ReflectionDebugger
	{
		bool open = false;

		Type* selectedType = nullptr;
		ImGuiTextFilter filter;
		TypeCategory categoryFilter = TypeCategory::All;


		ReflectionDebugger();

		void Draw();

	private:
		void DrawType(Type* type);
	};
}    // namespace rift::editor
