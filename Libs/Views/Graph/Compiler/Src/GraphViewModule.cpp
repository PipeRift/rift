// Copyright 2015-2024 Piperift - All rights reserved

#include "GraphViewModule.h"


namespace rift
{
	void GraphViewModule::Load()
	{
		RegisterView(View{
		    .name           = "Graph",
		    .supportedTypes = {ASTModule::classType, ASTModule::structType},
		    .onDrawEditor   = &GraphViewModule::DrawEditor
        });
	}
}    // namespace rift