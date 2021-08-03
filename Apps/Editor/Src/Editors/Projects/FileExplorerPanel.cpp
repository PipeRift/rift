// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/Projects/FileExplorerPanel.h"

#include "Editor.h"
#include "Editors/ProjectEditor.h"
#include "UI/Style.h"
#include "UI/UI.h"
#include "Uniques/CEditorUnique.h"

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
		ZoneScoped;
		// if (bDirty)
		{
			CacheProjectFiles(ast);
		}

		UI::BeginChild("Files");
		if (UI::BeginPopupContextWindow())
		{
			DrawContextMenu(ast, {}, AST::NoId);
			UI::EndPopup();
		}


		{
			ZoneScopedN("Draw File Explorer");
			for (const auto& item : folders[Name::None()].items)
			{
				DrawItem(ast, item);
			}
		}
		UI::EndChild();
	}

	void FileExplorerPanel::DrawContextMenu(AST::Tree& ast, StringView path, AST::Id itemId)
	{
		if (itemId != AST::NoId)
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

	void FileExplorerPanel::InsertItem(TMap<Name, Folder>& folders, const Item& item)
	{
		Name lastName         = item.path;
		StringView parentPath = Paths::GetParent(item.path.ToString());
		Name parentName{parentPath};

		if (Folder* parent = folders.Find(parentName))
		{
			parent->items.Add(item);
			return;
		}
		folders.Insert(parentName, Folder{{item}});

		parentPath = Paths::GetParent(parentPath);
		while (!parentPath.empty())
		{
			lastName   = parentName;
			parentName = Name{parentPath};

			const Item newItem{AST::NoId, lastName, true};
			if (Folder* parent = folders.Find(parentName))
			{
				parent->items.Add(newItem);
				// Found an existing folder. Leave since we assume all parent folders are valid
				return;
			}
			folders.Insert(parentName, Folder{{newItem}});

			parentPath = Paths::GetParent(parentPath);
		}
	}

	void FileExplorerPanel::CacheProjectFiles(AST::Tree& ast)
	{
		ZoneScopedN("Cache Project Files");
		bDirty = false;

		folders.Empty();

		// Set root folder (not displayed)
		folders.InsertDefaulted(Name::None());

		projectModuleId = Modules::GetProjectModule(ast);

		// Create module folders
		auto modules = ast.MakeView<CModule>();
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
					if (Strings::Contains<Path::value_type>(one.path.native(),
					        other.path.native()))    // Is relative
					{
						// If a module is inside another, create the folders in between
						InsertItem(folders, Item{oneId, path, true});
						insideOther = true;    // break;?
					}
				}
			}

			if (!insideOther)
			{
				folders[Name::None()].items.Add(Item{oneId, Name{path}, true});
			}
		}

		// Create items
		auto types = ast.MakeView<CType>();
		for (AST::Id typeId : types)
		{
			CType& type = types.Get<CType>(typeId);
			if (!type.path.empty())
			{
				const String path = Paths::ToString(type.path);
				InsertItem(folders, Item{typeId, Name{path}});
			}
		}
	}

	void FileExplorerPanel::DrawItem(AST::Tree& ast, const Item& item)
	{
		if (Folder* folder = folders.Find(item.path))
		{
			auto* module = item.id != AST::NoId ? ast.TryGet<CModule>(item.id) : nullptr;
			if (module)
			{
				// TODO: Display module name
				const StringView name = Paths::GetFilename(item.path.ToString());

				const bool isProject = item.id == projectModuleId;
				Style::PushHeaderColor(Style::primaryColor);
				Style::PushStyleCompact();
				bool open = UI::CollapsingHeader(
				    name.data(), isProject ? ImGuiTreeNodeFlags_DefaultOpen : 0);
				Style::PopStyleCompact();
				Style::PopHeaderColor();

				if (UI::BeginPopupContextItem())
				{
					DrawContextMenu(ast, item.path.ToString(), item.id);
					UI::EndPopup();
				}

				if (open)
				{
					UI::TreePush(name.data());

					// Sort items. FIrst folders, then alphabetically
					folder->items.Sort([](const auto& one, const auto& other) {
						if (one.isFolder != other.isFolder)
						{
							return one.isFolder;
						}
						return one.path.ToString() < other.path.ToString();
					});

					for (auto& childItem : folder->items)
					{
						DrawItem(ast, childItem);
					}
					UI::TreePop();
				}
				DrawModuleActions(item.id, *module);
			}
			else    // Just a folder
			{
				const StringView name = Paths::GetFilename(item.path.ToString());
				bool open             = UI::TreeNode(name.data());
				if (UI::BeginPopupContextItem())
				{
					DrawContextMenu(ast, item.path.ToString(), item.id);
					UI::EndPopup();
				}
				if (open)
				{
					for (auto& childItem : folder->items)
					{
						DrawItem(ast, childItem);
					}
					UI::TreePop();
				}
			}
		}
		else
		{
			StringView name = Paths::GetFilename(item.path.ToString());
			UI::TreeNodeEx(
			    name.data(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
			if (UI::BeginPopupContextItem())
			{
				DrawContextMenu(ast, item.path.ToString(), item.id);
				UI::EndPopup();
			}
		}
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

	void FileExplorerPanel::DrawModuleActions(AST::Id id, CModule& module) {}
	void FileExplorerPanel::DrawTypeActions(AST::Id id, CType& type) {}
}    // namespace Rift
