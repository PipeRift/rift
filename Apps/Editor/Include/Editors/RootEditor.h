// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "ProjectEditor.h"
#include "Tools/ASTDebugger.h"
#include "Tools/BigBestFitArenaDebugger.h"


namespace Rift
{
	struct RootEditor
	{
	private:
		ObjectPtr<ProjectEditor> projectEditor;
		Rift::BigBestFitArenaDebugger memoryDebugger;
		Rift::ASTDebugger astDebugger;

#if BUILD_DEBUG
		bool showDemo = false;
#endif


	public:
		bool OpenProject(Path path);
		void CloseProject();

		void Draw();

	private:
		void DrawMenuBar();
		void DrawProjectPickerPopup();
	};
}    // namespace Rift
