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

		struct Item
		{
			AST::Id id;
			Name path;
			bool isModule = false;
		};

		struct Folder
		{
			TArray<Item> items;
		};

	private:
		TMap<Name, Folder> folders;

		bool bOpen  = true;
		bool bDirty = true;

		Filter filter = Filter::All;


	public:
		FileExplorerPanel() {}

		void BuildLayout();
		void Draw(AST::Tree& ast);

		void DrawList(AST::Tree& ast);

		void DrawContextMenu(AST::Tree& ast, Path path, Item* item);

		void CacheProjectFiles(AST::Tree& ast);

	private:
		void CreateParentFolders(TMap<Name, Folder>& folders, StringView parentPath);
		void DrawItem(AST::Tree& ast, const Item& item);
		// void DrawFile(AST::Tree& ast, File& file);

		void CreateAsset(AST::Tree& ast, StringView title, TypeAsset::Type type, Path path);
	};


	inline const u32 operator*(FileExplorerPanel::Filter filter)
	{
		return static_cast<u32>(filter);
	}
}    // namespace Rift
