// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include <PipeReflect.h>


namespace rift::editor
{
	struct EditorSettings
	{
		P_STRUCT(EditorSettings)

		P_PROP(recentProjects)
		p::TArray<p::String> recentProjects;
	};
}    // namespace rift::editor
