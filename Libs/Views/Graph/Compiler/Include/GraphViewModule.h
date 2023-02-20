// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include <Compiler/Backend.h>
#include <FrameworkModule.h>
#include <Module.h>
#include <Pipe/Core/EnumFlags.h>
#include <View.h>


namespace rift
{
	class GraphViewModule : public Module
	{
		CLASS(GraphViewModule, Module)

	public:
		void Load() override;

		static void DrawEditor() {}
	};
}    // namespace rift
