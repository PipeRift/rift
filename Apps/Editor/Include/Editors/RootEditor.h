// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "ProjectEditor.h"


struct RootEditor
{
private:
	ObjectPtr<ProjectEditor> projectEditor;


public:
	bool OpenProject(Path path);
	void CloseProject();

	void Draw();

private:
	void DrawProjectPickerPopup();
};
