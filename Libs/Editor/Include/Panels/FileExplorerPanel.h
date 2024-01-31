// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Components/CProject.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CNamespace.h>
#include <AST/Components/Declarations.h>
#include <AST/Tree.h>
#include <Pipe/Core/String.h>
#include <PipeArrays.h>
#include <PipeECS.h>



namespace rift::Editor
{
	// Forward declarations
	class ProjectEditor;

	class FileExplorerPanel
	{
	public:
		enum class Filter : p::u32
		{
			None              = 0,
			Classes           = 1 << 0,
			Structs           = 1 << 1,
			FunctionLibraries = 1 << 2,
			All               = UINT_MAX
		};

		struct Item
		{
			AST::Id id = AST::NoId;
			p::String path;
			bool isFolder = false;
		};

		struct Folder
		{
			p::TArray<Item> items;
		};

	private:
		AST::Id projectModuleId = AST::NoId;
		p::TMap<p::Tag, Folder> folders;

		bool open  = true;
		bool dirty = true;

		Filter filter    = Filter::All;
		AST::Id renameId = AST::NoId;
		p::String renameBuffer;
		bool renameHasFocused = false;

		p::StringView pendingOpenCreatedPath;


	public:
		FileExplorerPanel() {}

		void BuildLayout();
		void Draw(AST::Tree& ast);

		void DrawList(AST::Tree& ast);

		void DrawContextMenu(AST::Tree& ast, p::StringView path, AST::Id itemId);

		void CacheProjectFiles(
		    p::TAccessRef<AST::CProject, AST::CModule, AST::CFileRef, AST::CDeclType> access);

		void SortFolder(Folder& folder);

	private:
		void InsertItem(const Item& item);
		void DrawItem(AST::Tree& ast, const Item& item);
		// void DrawFile(AST::Tree& ast, File& file);

		void DrawModuleActions(AST::Id id, struct AST::CModule& module);
		void DrawTypeActions(AST::Id id, struct AST::CDeclType& type);

		void CreateType(AST::Tree& ast, p::StringView title, p::Tag typeId, p::StringView path);
	};


	inline const p::u32 operator*(FileExplorerPanel::Filter filter)
	{
		return static_cast<p::u32>(filter);
	}
}    // namespace rift::Editor
