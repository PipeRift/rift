// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/EnumFlags.h"

#include <Compiler/Backend.h>
#include <Plugin.h>
#include <RiftContext.h>
#include <View.h>


namespace Rift
{
	using namespace EnumOperators;


	class GraphViewPlugin : public Plugin
	{
		CLASS(GraphViewPlugin, Plugin)

	public:
		void Register(TPtr<RiftContext> context) override
		{
			context->AddView(View{.name = "Graph",
			    .supportedTypes = Type::Class | Type::FunctionLibrary | Type::FunctionInterface,
			    .onDrawEditor   = &GraphViewPlugin::DrawEditor});
		}

		static void DrawEditor() {}
	};
}    // namespace Rift
