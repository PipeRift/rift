// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "ProjectEditor.h"
#include "Tools/BigBestFitArenaDebugger.h"


namespace Rift
{
	struct RootEditor
	{
	private:
		TOwnPtr<ProjectEditor> projectEditor = Create<ProjectEditor>();
		Rift::BigBestFitArenaDebugger memoryDebugger;

#if BUILD_DEBUG
		bool showDemo = false;
#endif


	public:
		void Draw();

	private:
		void DrawMenuBar();
		void DrawProjectPickerPopup();
	};
}    // namespace Rift
