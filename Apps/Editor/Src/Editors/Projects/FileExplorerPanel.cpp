// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/Projects/FileExplorerPanel.h"

#include "Editors/ProjectEditor.h"

#include <imgui.h>
#include <imgui_internal.h>


void FileExplorerPanel::Draw()
{
	editor.layout.BindNextWindowToNode(editor.leftNode);
	if (ImGui::Begin(
	        "File Explorer", &bOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
	{
		if (!editor.HasProject())
		{
			ImGui::Text("No active project.");
		}
		else
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Filter"))
				{
					static bool bClasses   = true;
					static bool bStructs   = true;
					static bool bFunctions = true;
					ImGui::Checkbox("Classes", &bClasses);
					ImGui::Checkbox("Structs", &bStructs);
					ImGui::Checkbox("Function Libraries", &bFunctions);
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			DrawList();
		}
	}
	ImGui::End();
}

void FileExplorerPanel::DrawList()
{
	if (bDirty)
	{
		CacheProjectFiles();
	}

	ImGui::BeginChild("Files");
	
	CreateFolderNode(projectFolder);

	ImGui::EndChild();
}

void FileExplorerPanel::CacheProjectFiles()
{
	bDirty = false;
	editor.project->ScanAssets();

	// TODO: the name of the project should be the same as the one in the project file
	projectFolder.name = "Project Name";
	
	OrganizeProjectFiles();
}

void FileExplorerPanel::OrganizeProjectFiles()
{
	for(auto& asset : editor.project->GetAllTypeAssets())
	{
		const Path path = FileSystem::FromString(asset.GetStrPath());
		const Path relative = FileSystem::ToRelative(path, editor.project->GetPath());
		const String relativeString = FileSystem::ToString(relative);
		Folder* current = &projectFolder;
		
		for(auto it = relative.begin(); it != relative.end(); ++it)
		{
			// TODO: move the file extensions to a config file or something
			static StringView extension = ".rf";

			const String name = FileSystem::ToString(*it);
			if(CString::EndsWith(name, extension))
			{
				current->files.Add({name, asset});
			}
			else
			{
				bool exists = false;
				for(i32 i = 0; i < current->folders.Size(); ++i)
				{
					Folder& folder = current->folders[i];
					if(folder.name == name)
					{
						current = &folder;
						exists = true;
						break;
					}
				}

				if(!exists)
				{
					current->folders.Add({name});
					current = &current->folders.Last();
				}
			}
		}

	}

}

void FileExplorerPanel::CreateFolderNode(const Folder& folder)
{
	if(ImGui::TreeNode(folder.name.c_str()))
	{
		for(auto& childFolder : folder.folders)
		{
			CreateFolderNode(childFolder);
		}

		for(auto& file : folder.files)
		{
			CreateFileNode(file);
		}

		ImGui::TreePop();
	}
}

void FileExplorerPanel::CreateFileNode(const File& file)
{
	ImGui::Selectable(file.name.c_str());
}
