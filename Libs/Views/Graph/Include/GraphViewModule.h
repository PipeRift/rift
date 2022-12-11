// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Compiler/Backend.h>
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
			RegisterView(View{.name = "Graph",
			    .supportedTypes     = AST::RiftType::Class | AST::RiftType::Static,
			    .onDrawEditor       = &GraphViewModule::DrawEditor});
		}

		static void DrawEditor() {}
	};
}    // namespace rift
