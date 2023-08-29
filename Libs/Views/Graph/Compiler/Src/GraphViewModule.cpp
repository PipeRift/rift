// Copyright 2015-2023 Piperift - All rights reserved

#include "GraphViewModule.h"

// P_OVERRIDE_NEW_DELETE

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