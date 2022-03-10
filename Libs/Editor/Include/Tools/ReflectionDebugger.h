// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"

#include <UI/UI.h>


namespace Rift
{
	struct ReflectionDebugger
	{
		bool open = false;

		Refl::Type* selectedType = nullptr;
		ImGuiTextFilter filter;
		Refl::TypeCategory categoryFilter = Refl::TypeCategory::All;


		ReflectionDebugger();

		void Draw();

	private:
		void DrawType(Refl::Type* type);
	};
}    // namespace Rift
