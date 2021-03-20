// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Assets/AssetPtr.h"
#include "Assets/TypeAsset.h"
#include "Containers/Array.h"
#include "Strings/String.h"

using namespace Rift;

// Forward declarations
class ProjectEditor;

class FileExplorerPanel
{
	struct File
	{
		String name;
		TAssetPtr<TypeAsset> info;
	};

	struct Folder
	{
		String name;
		TArray<Folder> folders;
		TArray<File> files;
	};

	ProjectEditor& editor;
	Folder projectFolder;
	bool bOpen  = true;
	bool bDirty = true;


public:
	FileExplorerPanel(ProjectEditor& editor) : editor(editor) {}

	void BuildLayout();
	void Draw();

	void DrawList();

	void CacheProjectFiles();

private:
	void OrganizeProjectFiles();
	void DrawFolderItems(const Folder& folder);
	void DrawFile(const File& file);
};
