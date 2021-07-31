// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/Projects/FileExplorerPanel.h"

#include "Editor.h"
#include "Editors/ProjectEditor.h"
#include "UI/Style.h"
#include "UI/UI.h"
#include "Uniques/CEditorUnique.h"

#include <AST/Components/CIdentifier.h>
#include <AST/Components/CModule.h>
#include <AST/Components/CType.h>
#include <AST/Linkage.h>
#include <AST/Utils/ModuleUtils.h>
#include <Files/FileDialog.h>
#include <Framework/Paths.h>
#include <GLFW/glfw3.h>
#include <RiftContext.h>
#include <Strings/FixedString.h>
#include <imgui_internal.h>


namespace Rift
{
	void FileExplorerPanel::Draw(AST::Tree& ast)
	{
		CEditorUnique& editor = ast.GetUnique<CEditorUnique>();
		editor.layout.BindNextWindowToNode(editor.leftNode);
		if (UI::Begin(
		        "File Explorer", &bOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
		{
			if (UI::BeginMenuBar())
			{
				if (UI::BeginMenu("Filter"))
				{
					UI::CheckboxFlags("Classes", (u32*) &filter, *Filter::Classes);
					UI::CheckboxFlags("Structs", (u32*) &filter, *Filter::Structs);
					UI::CheckboxFlags(
					    "Function Libraries", (u32*) &filter, *Filter::FunctionLibraries);
					UI::EndMenu();
				}
				UI::EndMenuBar();
			}
			DrawList(ast);
		}
		UI::End();
	}

	void FileExplorerPanel::DrawList(AST::Tree& ast)
	{
		// if (bDirty)
		{
			CacheProjectFiles(ast);
		}

		UI::BeginChild("Files");
		if (UI::BeginPopupContextWindow())
		{
			DrawContextMenu(ast, {}, {});
			UI::EndPopup();
		}


		for (const auto& item : folders[Name::None()].items)
		{
			DrawItem(ast, item);
		}
		UI::EndChild();
	}

	void FileExplorerPanel::DrawContextMenu(AST::Tree& ast, Path path, Item* item)
	{
		if (item)
		{
			if (UI::MenuItem("Rename"))
			{
				// file->renaming = true;
			}
			if (UI::MenuItem("Delete")) {}
		}
		else
		{
			if (UI::BeginMenu("Create"))
			{
				if (UI::MenuItem("Class"))
				{
					CreateAsset(ast, "Create Class file", TypeAsset::Type::Class, path);
				}
				if (UI::MenuItem("Struct"))
				{
					CreateAsset(ast, "Create Struct file", TypeAsset::Type::Struct, path);
				}
				if (UI::MenuItem("Function Library"))
				{
					CreateAsset(ast, "Create Function Library file",
					    TypeAsset::Type::FunctionLibrary, path);
				}
				UI::EndMenu();
			}
		}
	}

	void FileExplorerPanel::CreateParentFolders(TMap<Name, Folder>& folders, StringView parentPath)
	{
		Name lastName{parentPath};
		parentPath = Paths::GetParent(parentPath);
		Name parentName{parentPath};
		while (!parentPath.empty())
		{
			if (Folder* parent = folders.Find(parentName))
			{
				parent->items.Add({AST::NoId, lastName});
				// Found an existing folder. Leave since we assume all parent folders are valid
				return;
			}
			else
			{
				folders.Insert(parentName, Folder{{Item{AST::NoId, lastName}}});

				lastName   = parentName;
				parentPath = Paths::GetParent(parentPath);
				parentName = Name{parentPath};
			}
		}
	}

	void FileExplorerPanel::CacheProjectFiles(AST::Tree& ast)
	{
		bDirty = false;

		folders.Empty();

		// Set root folder (not displayed)
		folders.InsertDefaulted(Name::None());

		auto modules = ast.MakeView<CIdentifier, CModule>();

		AST::Id projectModId = Modules::GetProjectModule(ast);
		auto& projectModule  = modules.Get<CModule>(projectModId);

		// Create module folders
		for (AST::Id moduleId : modules)
		{
			auto& mod = modules.Get<CModule>(moduleId);
			const Name pathName{Paths::ToString(mod.path)};
			folders.InsertDefaulted(pathName);
		}

		// Create folders between modules
		for (AST::Id oneId : modules)
		{
			bool insideOther = false;
			auto& one        = modules.Get<CModule>(oneId);

			const String path = Paths::ToString(one.path);
			for (AST::Id otherId : modules)
			{
				if (oneId != otherId)
				{
					auto& other = modules.Get<CModule>(otherId);
					if (Paths::IsInside(one.path, other.path))
					{
						// If a module is inside another, create the folders in between
						CreateParentFolders(folders, path);
						insideOther = true;
					}
				}
			}

			if (!insideOther)
			{
				folders[Name::None()].items.Add(Item{oneId, Name{path}});
			}
		}

		// Create items
		auto types = ast.MakeView<CType>();
		for (AST::Id typeId : types)
		{
			CType& type = types.Get<CType>(typeId);
			if (!type.path.empty())
			{
				CreateParentFolders(folders, Paths::ToString(type.path));
			}
		}
	}

	/*TArray<Folder> moduleFolders;
	auto types = ast.MakeView<CType>();
	for (AST::Id moduleId : modules)
	{
	    Folder& folder = moduleFolders.AddDefaultedRef();
	    folder.name    = Modules::GetModuleName(ast, moduleId).ToString();

	    TArray<AST::Id>* linked = AST::GetLinked(ast, moduleId);
	    TArray<TPair<AST::Id, CType*>> types;
	    types.Reserve(linked.Size());
	    for (AST::Id link : linked)
	    {
	        if (CType* type = types.TryGet<CType>(link))
	        {
	            types.Add(type);
	        }
	    }
	    // Remove non types

	    linked.Sort([&types](AST::Id oneId, AST::Id otherId) {
	        CType& one   = types.Get<CType>(oneId);
	        CType& other = types.Get<CType>(otherId);

	        return one.path > other.path;
	    });

	    TFixedString<512> currentPath;
	    for (AST::Id type : types)
	    {
	        while ()
	    }
	}

	for (Folder& folder : moduleFolders) {}

	// Sort
	for (Folder& folder : moduleFolders) {}
	*/

	// project->ScanAssets();

	// Reset cached data
	// projectFolder = {};

	/*for (auto& asset : project->GetAllTypeAssets())
	{
	    const Path path     = Paths::FromString(asset.GetStrPath());
	    const Path relative = Paths::ToRelative(path, project->GetPath());
	    Folder* current     = &projectFolder;

	    for (auto it = relative.begin(); it != relative.end(); ++it)
	    {
	        const String name = Paths::ToString(*it);
	        if (Strings::EndsWith(name, Paths::typeExtension))
	        {
	            current->files.Add({name, asset});
	        }
	        else
	        {
	            bool exists = false;
	            for (i32 i = 0; i < current->folders.Size(); ++i)
	            {
	                Folder& folder = current->folders[i];
	                if (folder.name == name)
	                {
	                    current = &folder;
	                    exists  = true;
	                    break;
	                }
	            }

	            if (!exists)
	            {
	                current->folders.Add({name});
	                current = &current->folders.Last();
	            }
	        }
	    }
	}
	*/

	void FileExplorerPanel::DrawItem(AST::Tree& ast, const Item& item)
	{
		if (Folder* folder = folders.Find(item.path))
		{
			// TODO: Display module name
			StringView name = Paths::GetFilename(item.path.ToString());
			if (UI::TreeNode(name.data()))
			{
				for (auto& childItem : folder->items)
				{
					DrawItem(ast, childItem);
				}
				UI::TreePop();
			}
		}
		else
		{
			StringView name = Paths::GetFilename(item.path.ToString());
			UI::TreeNodeEx(
			    name.data(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		}


		/*if (UI::TreeNode(folder.name.c_str()))
		{
		    for (auto& item : folder.items)
		    {
		        if (UI::TreeNode(childFolder.name.c_str()))
		        {
		            DrawFolderItems(ast, childFolder);
		            UI::TreePop();
		        }
		        if (UI::BeginPopupContextItem())
		        {
		            const Path path = Paths::FromString(childFolder.name);
		            DrawContextMenu(ast, path, nullptr);
		            UI::EndPopup();
		        }
		    }

		    for (auto& file : folder.files)
		    {
		        DrawFile(ast, file);
		    }
		    UI::TreePop();
		}*/
	}

	/*void FileExplorerPanel::DrawFile(AST::Tree& ast, File& file)
	{
	    if (!file.renaming)
	    {
	        UI::TreeNodeEx(
	            file.name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
	        if (UI::IsItemHovered() && UI::IsKeyReleased(GLFW_KEY_F2))
	        {
	            file.renaming = true;
	        }
	    }
	    else
	    {
	        UI::Indent();
	        Style::PushStyleCompact();
	        // TODO: Implement UI functions for string types
	        TFixedString<128> buffer;

	        UI::SetKeyboardFocusHere();
	        if (UI::InputText("##newname", buffer.data(), buffer.size(),
	                ImGuiInputTextFlags_EnterReturnsTrue))
	        {
	            file.renaming = false;
	        }
	        // if (!UI::IsItemActive())
	        //{
	        //	file.renaming = false;
	        //}
	        Style::PopStyleCompact();
	        UI::Unindent();
	    }


	    if (UI::IsItemClicked())
	    {
	        if (UI::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	        {
	            // editor.OpenType(file.info);
	        }
	    }

	    if (UI::BeginPopupContextItem())
	    {
	        const Path path = Paths::FromString(file.info.GetStrPath());
	        DrawContextMenu(ast, path, &file);
	        UI::EndPopup();
	    }
	}*/


	void FileExplorerPanel::CreateAsset(
	    AST::Tree& ast, StringView title, TypeAsset::Type type, Path path)
	{
		const Path filename = Dialogs::SaveFile(
		    title, path, {{"Rift file", Strings::Format("*.{}", Paths::typeExtension)}}, true);
		TAssetPtr<TypeAsset> newAsset{filename};
		if (newAsset.LoadOrCreate())
		{
			newAsset->type = type;
			newAsset->InitializeDeclaration(ast);
			newAsset->Save();
			bDirty = true;
		}
		else
		{
			const String filenameStr = Paths::ToString(filename);
			Log::Error("Couldn't create file '{}'", filenameStr);
		}
	}
}    // namespace Rift
