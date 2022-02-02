// Copyright 2015-2022 Piperift - All rights reserved

#include "Panels/FileExplorerPanel.h"

#include "Editor.h"
#include "Files/Files.h"
#include "Files/Paths.h"
#include "Files/STDFileSystem.h"
#include "Statics/SEditor.h"
#include "Strings/StringView.h"
#include "UI/Widgets.h"
#include "Utils/TypeUtils.h"

#include <AST/Components/CFileRef.h>
#include <AST/Components/CIdentifier.h>
#include <AST/Components/CModule.h>
#include <AST/Utils/Hierarchy.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/TypeUtils.h>
#include <Files/FileDialog.h>
#include <Framework/Paths.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <imgui_internal.h>
#include <RiftContext.h>
#include <Strings/FixedString.h>
#include <UI/Style.h>
#include <UI/UI.h>


namespace Rift
{
	void FileExplorerPanel::Draw(AST::Tree& ast)
	{
		auto& editor = ast.GetStatic<SEditor>();
		editor.layout.BindNextWindowToNode(SEditor::leftNode);

		if (UI::Begin(
		        "File Explorer", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar))
		{
			if (UI::BeginMenuBar())
			{
				if (UI::BeginMenu("Filter"))
				{
					UI::CheckboxFlags("Classes", (u32*)&filter, *Filter::Classes);
					UI::CheckboxFlags("Structs", (u32*)&filter, *Filter::Structs);
					UI::CheckboxFlags(
					    "Function Libraries", (u32*)&filter, *Filter::FunctionLibraries);
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
		// if (dirty)
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
			for (auto& item : folders[Name::None()].items)
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
			if (ast.Has<CType>(itemId))
			{
				if (UI::MenuItem("Open"))
				{
					Types::OpenType(ast, itemId);
				}
			}

			if (UI::MenuItem("Rename"))
			{
				renameId     = itemId;
				renameBuffer = Paths::GetFilename(path);
			}
			if (UI::MenuItem("Delete")) {}
		}
		else
		{
			if (UI::BeginMenu("Create"))
			{
				if (UI::MenuItem("Class"))
				{
					CreateType(ast, "Create Class file", TypeCategory::Class, path);
				}
				if (UI::MenuItem("Struct"))
				{
					CreateType(ast, "Create Struct file", TypeCategory::Struct, path);
				}
				if (UI::MenuItem("Function Library"))
				{
					CreateType(
					    ast, "Create Function Library file", TypeCategory::FunctionLibrary, path);
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
		dirty = false;

		folders.Empty();

		// Set root folder (not displayed)
		folders.InsertDefaulted(Name::None());

		projectModuleId = Modules::GetProjectId(ast);

		// Create module folders
		auto modules = ast.Filter<CModule>();
		TMap<AST::Id, Path> moduleFolders;
		moduleFolders.Reserve(u32(modules.Size()));
		for (AST::Id moduleId : modules)
		{
			auto& file      = ast.Get<CFileRef>(moduleId);
			Path folderPath = file.path.parent_path();
			const Name pathName{Paths::ToString(folderPath)};
			folders.InsertDefaulted(pathName);
			moduleFolders.Insert(moduleId, Move(folderPath));
		}

		// Create folders between modules
		for (AST::Id oneId : modules)
		{
			bool insideOther = false;
			const Path& path = moduleFolders[oneId];

			for (AST::Id otherId : modules)
			{
				if (oneId != otherId)
				{
					const Path& otherPath = moduleFolders[otherId];

					if (Strings::Contains<Path::value_type>(path.native(),
					        otherPath.native()))    // Is relative
					{
						insideOther = true;
						break;
					}
				}
			}

			if (insideOther)
			{
				// If a module is inside another, create the folders in between
				InsertItem(folders, Item{oneId, Paths::ToString(path), true});
			}
			else
			{
				folders[Name::None()].items.Add(Item{oneId, Name{Paths::ToString(path)}, true});
			}
		}

		// Create items
		auto fileTypes = ast.Filter<CType, CFileRef>();
		for (AST::Id typeId : fileTypes)
		{
			auto& file = fileTypes.Get<CFileRef>(typeId);
			if (!file.path.empty())
			{
				const String path = Paths::ToString(file.path);
				InsertItem(folders, Item{typeId, Name{path}});
			}
		}
	}

	void FileExplorerPanel::SortFolder(Folder& folder)
	{
		// Sort items. First folders, then alphabetically
		folder.items.Sort([](const auto& one, const auto& other) {
			if (one.isFolder != other.isFolder)
			{
				return one.isFolder;
			}
			return one.path.ToString() < other.path.ToString();
		});
	}

	void FileExplorerPanel::DrawItem(AST::Tree& ast, const Item& item)
	{
		const String path         = item.path.ToString();
		const StringView fileName = Paths::GetFilename(path);

		if (Folder* folder = folders.Find(item.path))
		{
			ImGuiTreeNodeFlags flags = 0;
			if (folder->items.IsEmpty())
			{
				flags |= ImGuiTreeNodeFlags_Bullet;
			}

			auto* module = item.id != AST::NoId ? ast.TryGet<CModule>(item.id) : nullptr;
			if (module)
			{
				// TODO: Display module name
				const String text = Strings::Format(ICON_FA_BOX " {}", fileName);

				Style::PushHeaderColor(Style::primaryColor);
				Style::PushStyleCompact();
				const bool isProject = item.id == projectModuleId;
				if (item.id == projectModuleId)    // Is project
				{
					flags |= ImGuiTreeNodeFlags_DefaultOpen;
				}
				bool open = UI::CollapsingHeader(text.data(), flags);
				Style::PopStyleCompact();
				Style::PopHeaderColor();

				if (UI::BeginPopupContextItem())
				{
					DrawContextMenu(ast, path, item.id);
					UI::EndPopup();
				}

				if (open)
				{
					UI::TreePush(text.data());

					SortFolder(*folder);
					for (const auto& childItem : folder->items)
					{
						DrawItem(ast, childItem);
					}
					UI::TreePop();
				}
				DrawModuleActions(item.id, *module);
			}
			else    // Just a folder
			{
				const String text = Strings::Format(ICON_FA_FOLDER " {}", fileName);
				bool open         = UI::TreeNodeEx(text.data(), flags);
				if (UI::BeginPopupContextItem())
				{
					DrawContextMenu(ast, path, item.id);
					UI::EndPopup();
				}
				if (open)
				{
					SortFolder(*folder);
					for (const auto& childItem : folder->items)
					{
						DrawItem(ast, childItem);
					}
					UI::TreePop();
				}
			}
		}
		else
		{
			String text;
			if (Strings::EndsWith(fileName, ".rf"))
			{
				text = Strings::Format(ICON_FA_FILE_CODE " {}", fileName.data());
			}
			else if (Strings::EndsWith(fileName, ".rift"))
			{
				text = Strings::Format(ICON_FA_FILE_ALT " {}", fileName);
			}
			else
			{
				text = fileName;
			}

			if (renameId != item.id)
			{
				UI::TreeNodeEx(
				    text.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
				if (UI::IsItemHovered() && UI::IsKeyReleased(GLFW_KEY_F2))
				{
					renameId     = item.id;
					renameBuffer = fileName;
				}
			}
			else
			{
				UI::Indent();
				if (!renameHasFocused)
				{
					renameHasFocused = true;
					UI::SetKeyboardFocusHere();
				}
				UI::InputText("##newname", renameBuffer, ImGuiInputTextFlags_AutoSelectAll);
				if (UI::IsItemDeactivatedAfterEdit())
				{
					renameId         = AST::NoId;
					Path destination = Paths::FromString(Paths::GetParent(path)) / renameBuffer;
					destination.replace_extension("rf");
					// FIX: Type caches dont detect renamed types. Should provide helpers for that
					Files::Rename(Paths::FromString(path), destination);
					renameBuffer     = "";
					renameHasFocused = false;
				}
				else if (UI::IsItemDeactivated())
				{
					renameId         = AST::NoId;
					renameBuffer     = "";
					renameHasFocused = false;
				}
				UI::Unindent();
			}

			if (UI::BeginPopupContextItem())
			{
				DrawContextMenu(ast, path, item.id);
				UI::EndPopup();
			}


			if (UI::IsItemClicked())
			{
				if (UI::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					Types::OpenType(ast, item.id);
				}
			}

			if (Types::IsTypeOpen(ast, item.id))
			{
				UI::SameLine(ImGui::GetContentRegionAvailWidth(), 0);
				UI::Bullet();
				UI::NewLine();
			}
		}
	}

	void FileExplorerPanel::DrawModuleActions(AST::Id id, CModule& module) {}
	void FileExplorerPanel::DrawTypeActions(AST::Id id, CType& type) {}

	void FileExplorerPanel::CreateType(
	    AST::Tree& ast, StringView title, TypeCategory category, Path folderPath)
	{
		const Path path = Dialogs::SaveFile(title, folderPath,
		    {
		        {"Rift Type", Strings::Format("*.{}", Paths::typeExtension)}
        },
		    true);

		AST::Id typeId = Types::CreateType(ast, category);
		ast.Add<CFileRef>(typeId, path);

		String data;
		Types::Serialize(ast, typeId, data);
		Files::SaveStringFile(path, data);

		// Destroy the temporal type after saving it
		ast.Destroy(typeId);
	}
}    // namespace Rift