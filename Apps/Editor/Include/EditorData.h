// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Editors/ProjectEditor.h"
#include "Tools/BigBestFitArenaDebugger.h"

#include <Memory/OwnPtr.h>
#include <Object/Struct.h>


namespace Rift
{
	struct EditorData : public Struct
	{
		STRUCT(EditorData, Struct)

		TOwnPtr<ProjectEditor> projectEditor = Create<ProjectEditor>();

		Rift::BigBestFitArenaDebugger memoryDebugger;

#if BUILD_DEBUG
		bool showDemo = false;
#endif
	};
}    // namespace Rift
