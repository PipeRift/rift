// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <UI/UI.h>


namespace rift
{
	using namespace p::serl;
	using namespace p::refl;


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
}    // namespace rift
