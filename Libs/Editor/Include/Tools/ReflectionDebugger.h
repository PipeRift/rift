// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "AST/Tree.h"
#include "PipeReflect.h"

#include <UI/UI.h>


namespace rift::editor
{
	using namespace p;


	struct ReflectionDebugger
	{
		bool open = false;

		TypeId selectedType;
		ImGuiTextFilter filter;
		TypeFlags typeFlagsFilter = p::TF_Native | p::TF_Enum | p::TF_Struct | p::TF_Object;


		ReflectionDebugger();

		void Draw();

	private:
		void DrawType(TypeId type);
	};
}    // namespace rift::editor
