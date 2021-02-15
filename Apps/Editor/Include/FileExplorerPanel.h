// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

class ProjectEditor;
class FileExplorerPanel
{
	ProjectEditor& editor;
	bool bOpen  = true;
	bool bDirty = true;

public:
	FileExplorerPanel(ProjectEditor& editor) : editor(editor) {}
	void Draw();

	void DrawList();

	void CacheProjectFiles();
};
