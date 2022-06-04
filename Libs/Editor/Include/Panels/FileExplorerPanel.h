// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Components/CProject.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CType.h>
#include <AST/Tree.h>
#include <Core/Array.h>
#include <Core/String.h>
#include <ECS/Filtering.h>


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
			bool isFolder = false;
		};

		struct Folder
		{
			TArray<Item> items;
		};

	private:
		AST::Id projectModuleId = AST::NoId;
		TMap<Name, Folder> folders;

		bool open  = true;
		bool dirty = true;

		Filter filter    = Filter::All;
		AST::Id renameId = AST::NoId;
		String renameBuffer;
		bool renameHasFocused = false;

		Path pendingOpenCreatedPath;


	public:
		FileExplorerPanel() {}

		void BuildLayout();
		void Draw(AST::Tree& ast);

		void DrawList(AST::Tree& ast);

		void DrawContextMenu(AST::Tree& ast, StringView path, AST::Id itemId);

		void CacheProjectFiles(TAccessRef<CProject, CModule, CFileRef, CType> access);

		void SortFolder(Folder& folder);

	private:
		void InsertItem(TMap<Name, Folder>& folders, const Item& item);
		void DrawItem(AST::Tree& ast, const Item& item);
		// void DrawFile(AST::Tree& ast, File& file);

		void DrawModuleActions(AST::Id id, struct CModule& module);
		void DrawTypeActions(AST::Id id, struct CType& type);

		void CreateType(AST::Tree& ast, StringView title, Type category, Pipe::Path path);
	};


	inline const u32 operator*(FileExplorerPanel::Filter filter)
	{
		return static_cast<u32>(filter);
	}
}    // namespace Rift
