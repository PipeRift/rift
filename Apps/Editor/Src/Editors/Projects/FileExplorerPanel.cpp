// Copyright 2015-2021 Piperift - All rights reserved

#include "Editor.h"
#include "Editors/ProjectEditor.h"
#include "Editors/Projects/FileExplorerPanel.h"
#include "UI/Style.h"
#include "UI/UI.h"
#include "Uniques/CEditorUnique.h"

#include <AST/Components/CIdentifier.h>
#include <AST/Components/CModule.h>
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
		if (bDirty)
		{
			CacheProjectFiles(ast);
		}

		UI::BeginChild("Files");
		if (UI::BeginPopupContextWindow())
		{
			DrawContextMenu(ast, {}, {});
			UI::EndPopup();
		}

		auto modules = ast.MakeView<CIdentifier, CModule>();
		for (AST::Id moduleId : modules)
		{
			auto& ident = modules.Get<CIdentifier>(moduleId);
			// auto& mod   = modules.Get<CModule>(moduleId);
			if (UI::TreeNode(ident.name.ToString().c_str()))
			{
				UI::TreePop();
			}
		}

		DrawFolderItems(ast, projectFolder);

		UI::EndChild();
	}

	void FileExplorerPanel::DrawContextMenu(AST::Tree& ast, Path path, File* file)
	{
		if (file && file->info)
		{
			if (UI::MenuItem("Rename"))
			{
				file->renaming = true;
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

	void FileExplorerPanel::CacheProjectFiles(AST::Tree& ast)
	{
		bDirty = false;
		// CModule* mod = Modules::GetProjectModule(ast);
		// assert(mod);

		// project->ScanAssets();

		// Reset cached data
		projectFolder = {};

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
	}

	void FileExplorerPanel::DrawFolderItems(AST::Tree& ast, Folder& folder)
	{
		for (auto& childFolder : folder.folders)
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
	}

	void FileExplorerPanel::DrawFile(AST::Tree& ast, File& file)
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
	}


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
