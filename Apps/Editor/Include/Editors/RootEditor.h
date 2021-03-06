// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "ProjectEditor.h"
#include "Tools/BestFitArenaDebugger.h"


struct RootEditor
{
private:
	ObjectPtr<ProjectEditor> projectEditor;
	Rift::BestFitArenaDebugger memoryDebugger;


public:
	bool OpenProject(Path path);
	void CloseProject();

	void Draw();

private:
	void DrawMenuBar();
	void DrawProjectPickerPopup();
};
