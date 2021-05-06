// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Assets/AssetPtr.h"
#include "Assets/TypeAsset.h"

#include <Containers/Array.h>
#include <Strings/String.h>


namespace Rift
{
	// Forward declarations
	class ProjectEditor;

	class FileExplorerPanel
	{
	public:
		enum class Filter : u32
		{
			None              = 0,
			Classes           = 1 << 0,
			Structs           = 1 << 1,
			FunctionLibraries = 1 << 2,
			All               = UINT_MAX
		};

		struct File
		{
			String name;
			TAssetPtr<TypeAsset> info;
			bool renaming = false;
		};

		struct Folder
		{
			String name;
			TArray<Folder> folders;
			TArray<File> files;
		};

	private:
		ProjectEditor& editor;
		Folder projectFolder;
		bool bOpen  = true;
		bool bDirty = true;

		Filter filter = Filter::All;


	public:
		FileExplorerPanel(ProjectEditor& editor) : editor(editor) {}

		void BuildLayout();
		void Draw();

		void DrawList();

		void DrawContextMenu(Path path, File* file);

		void CacheProjectFiles();

	private:
		void DrawFolderItems(Folder& folder);
		void DrawFile(File& file);

		void CreateAsset(StringView title, TypeAsset::Type type, Path path);
	};


	inline const u32 operator*(FileExplorerPanel::Filter filter)
	{
		return static_cast<u32>(filter);
	}
}    // namespace Rift
