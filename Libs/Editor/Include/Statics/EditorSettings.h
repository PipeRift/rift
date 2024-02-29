// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>


namespace rift::editor
{
	struct EditorSettings : public p::Struct
	{
		P_STRUCT(EditorSettings, p::Struct)

		P_PROP(recentProjects)
		p::TArray<p::String> recentProjects;
	};
}    // namespace rift::editor
