// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include <Compiler/Backend.h>
#include <Pipe/Core/EnumFlags.h>
#include <Plugin.h>
#include <Rift.h>
#include <View.h>


namespace rift
{
	class GraphViewPlugin : public Plugin
	{
		CLASS(GraphViewPlugin, Plugin)

	public:
		void Register(TPtr<Rift> rift) override
		{
			rift->AddView(View{.name = "Graph",
			    .supportedTypes      = AST::RiftType::Class | AST::RiftType::Static,
			    .onDrawEditor        = &GraphViewPlugin::DrawEditor});
		}

		static void DrawEditor() {}
	};
}    // namespace rift
