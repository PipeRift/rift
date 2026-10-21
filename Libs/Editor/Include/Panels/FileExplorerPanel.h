// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "AST/Components/CProject.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CNamespace.h>
#include <AST/Components/Declarations.h>
#include <AST/Tree.h>
#include <PipeContainers.h>
#include <PipeECS.h>
#include <PipeStrings.h>


namespace rift::editor
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
			ast::Id id = ast::NoId;
			p::String path;
			bool isFolder = false;
		};

		struct Folder
		{
			p::TArray<Item> items;
		};

	private:
		ast::Id projectModuleId = ast::NoId;
		p::TMap<p::Tag, Folder> folders;

		bool open = true;

		Filter filter    = Filter::All;
		ast::Id renameId = ast::NoId;
		p::String renameBuffer;
		bool renameHasFocused = false;

		ast::Id pendingReloadId = ast::NoId;
		bool wantsReloadPopup   = false;

		p::StringView pendingOpenCreatedPath;


	public:
		FileExplorerPanel() {}

		void Draw(ast::Tree& ast);

		void DrawList(ast::Tree& ast);

		void DrawContextMenu(ast::Tree& ast, p::StringView path, ast::Id itemId);

		void CacheProjectFiles(
		    p::TIdScopeRef<ast::CProject, ast::CModule, ast::CFileRef, ast::CDeclType> scope);

		void SortFolder(Folder& folder);

	private:
		void InsertItem(const Item& item);
		void DrawItem(ast::Tree& ast, const Item& item);

		void DrawReloadPopup(ast::Tree& ast);
		void ReloadFile(ast::Tree& ast, ast::Id id);

		void CreateType(ast::Tree& ast, p::StringView title, p::Tag typeId, p::StringView path);
	};


	inline const p::u32 operator*(FileExplorerPanel::Filter filter)
	{
		return static_cast<p::u32>(filter);
	}
}    // namespace rift::editor
