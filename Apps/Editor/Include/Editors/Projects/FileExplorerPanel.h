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
		};

		struct Folder
		{
			TArray<Item> items;
		};

	private:
		AST::Id projectModuleId = AST::NoId;
		TMap<Name, Folder> folders;

		bool bOpen  = true;
		bool bDirty = true;

		Filter filter = Filter::All;


	public:
		FileExplorerPanel() {}

		void BuildLayout();
		void Draw(AST::Tree& ast);

		void DrawList(AST::Tree& ast);

		void DrawContextMenu(AST::Tree& ast, StringView path, AST::Id itemId);

		void CacheProjectFiles(AST::Tree& ast);

	private:
		void InsertItem(TMap<Name, Folder>& folders, const Item& item);
		void DrawItem(AST::Tree& ast, const Item& item);
		// void DrawFile(AST::Tree& ast, File& file);

		void CreateAsset(AST::Tree& ast, StringView title, TypeAsset::Type type, Path path);

		void DrawModuleActions(AST::Id id, struct CModule& module);
		void DrawTypeActions(AST::Id id, struct CType& type);
	};


	inline const u32 operator*(FileExplorerPanel::Filter filter)
	{
		return static_cast<u32>(filter);
	}
}    // namespace Rift
