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
		void Register() override
		{
			RegisterView(View{
			    .name           = "Graph",
			    .supportedTypes = {FrameworkModule::classType, FrameworkModule::structType},
			    .onDrawEditor   = &GraphViewModule::DrawEditor
            });
		}

		static void DrawEditor() {}
	};
}    // namespace rift
