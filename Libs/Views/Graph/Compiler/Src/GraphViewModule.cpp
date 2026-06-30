// Copyright 2015-2026 Piperift. All Rights Reserved.

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