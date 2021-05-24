// Copyright 2015-2021 Piperift - All rights reserved

#include "Editors/Projects/FileExplorerPanel.h"

#include "Editors/ProjectEditor.h"
#include "UI/Style.h"
#include "UI/UI.h"

#include <Files/FileDialog.h>
#include <GLFW/glfw3.h>
#include <RiftContext.h>
#include <imgui_internal.h>


namespace Rift
{
	void FileExplorerPanel::Draw()
	{
		editor.layout.BindNextWindowToNode(editor.leftNode);
		if (ImGui::Begin(
		        "File Explorer", &bOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Filter"))
				{
					ImGui::CheckboxFlags("Classes", (u32*) &filter, *Filter::Classes);
					ImGui::CheckboxFlags("Structs", (u32*) &filter, *Filter::Structs);
					ImGui::CheckboxFlags(
					    "Function Libraries", (u32*) &filter, *Filter::FunctionLibraries);
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			DrawList();
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
		if (ImGui::BeginPopupContextWindow())
		{
			DrawContextMenu({}, {});
			ImGui::EndPopup();
		}

		DrawFolderItems(projectFolder);

		ImGui::EndChild();
	}

	void FileExplorerPanel::DrawContextMenu(Path path, File* file)
	{
		if (file && file->info)
		{
			if (ImGui::MenuItem("Rename"))
			{
				file->renaming = true;
			}
			if (ImGui::MenuItem("Delete")) {}
		}
		else
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Class"))
				{
					CreateAsset("Create Class file", TypeAsset::Type::Class, path);
				}
				if (ImGui::MenuItem("Struct"))
				{
					CreateAsset("Create Struct file", TypeAsset::Type::Struct, path);
				}
				if (ImGui::MenuItem("Function Library"))
				{
					CreateAsset(
					    "Create Function Library file", TypeAsset::Type::FunctionLibrary, path);
				}
				ImGui::EndMenu();
			}
		}
	}

	void FileExplorerPanel::CacheProjectFiles()
	{
		bDirty       = false;
		auto project = RiftContext::GetProject();
		assert(project);

		project->ScanAssets();

		// Reset cached data
		projectFolder = {};

		for (auto& asset : project->GetAllTypeAssets())
		{
			const Path path     = Paths::FromString(asset.GetStrPath());
			const Path relative = Paths::ToRelative(path, project->GetPath());
			Folder* current     = &projectFolder;

			for (auto it = relative.begin(); it != relative.end(); ++it)
			{
				// TODO: move the file extensions to a config file or something
				static StringView extension = ".rf";

				const String name = Paths::ToString(*it);
				if (Strings::EndsWith(name, extension))
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
	}

	void FileExplorerPanel::DrawFolderItems(Folder& folder)
	{
		for (auto& childFolder : folder.folders)
		{
			if (ImGui::TreeNode(childFolder.name.c_str()))
			{
				DrawFolderItems(childFolder);
				ImGui::TreePop();
			}
			if (ImGui::BeginPopupContextItem())
			{
				const Path path = Paths::FromString(childFolder.name);
				DrawContextMenu(path, nullptr);
				ImGui::EndPopup();
			}
		}

		for (auto& file : folder.files)
		{
			DrawFile(file);
		}
	}

	void FileExplorerPanel::DrawFile(File& file)
	{
		if (!file.renaming)
		{
			ImGui::TreeNodeEx(
			    file.name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
			if (ImGui::IsItemHovered() && ImGui::IsKeyReleased(GLFW_KEY_F2))
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

			ImGui::SetKeyboardFocusHere();
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


		if (ImGui::IsItemClicked())
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				editor.OpenType(file.info);
			}
		}

		if (ImGui::BeginPopupContextItem())
		{
			const Path path = Paths::FromString(file.info.GetStrPath());
			DrawContextMenu(path, &file);
			ImGui::EndPopup();
		}
	}


	void FileExplorerPanel::CreateAsset(StringView title, TypeAsset::Type type, Path path)
	{
		const Path filename = Dialogs::SaveFile(title, path, {{"Rift file", "*.rf"}}, true);
		TAssetPtr<TypeAsset> newAsset{filename};
		if (newAsset.LoadOrCreate())
		{
			newAsset->type = type;
			newAsset->InitializeDeclaration(RiftContext::GetProject()->GetAST());
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
