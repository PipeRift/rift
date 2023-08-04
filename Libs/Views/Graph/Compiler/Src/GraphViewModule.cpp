// Copyright 2015-2023 Piperift - All rights reserved

#include "GraphViewModule.h"


namespace rift
{
	void GraphViewModule::Load()
	{
		RegisterView(View{
		    .name           = "Graph",
		    .supportedTypes = {FrameworkModule::classType, FrameworkModule::structType},
		    .onDrawEditor   = &GraphViewModule::DrawEditor
        });
	}
}    // namespace rift