// Copyright 2015-2023 Piperift - All rights reserved

#include "Panels/FileExplorerPanel.h"

#include "Editor.h"
#include "Statics/SEditor.h"
#include "UI/Widgets.h"
#include "Utils/ModuleUtils.h"
#include "Utils/TypeUtils.h"

#include <AST/Statics/STypes.h>
#include <AST/Utils/ModuleUtils.h>
#include <AST/Utils/Paths.h>
#include <AST/Utils/TransactionUtils.h>
#include <AST/Utils/TypeUtils.h>
#include <GLFW/glfw3.h>
#include <IconsFontAwesome5.h>
#include <imgui_internal.h>
#include <Pipe/Core/FixedString.h>
#include <Pipe/Core/PlatformProcess.h>
#include <Pipe/Core/StringView.h>
#include <Pipe/ECS/Utils/Hierarchy.h>
#include <Pipe/Files/FileDialog.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Files/STDFileSystem.h>
#include <UI/Style.h>
#include <UI/UI.h>


namespace rift::Editor
{
	void FileExplorerPanel::Draw(AST::Tree& ast)
	{
		// Open recently created types
		if (!pendingOpenCreatedPath.empty())
		{
			AST::Id typeId = AST::FindTypeByPath(ast, pendingOpenCreatedPath);
			if (!IsNone(typeId))
			{
				OpenType(ast, typeId);
				pendingOpenCreatedPath = p::Path{};
			}
		}


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
			String projectPath = p::ToString(AST::GetProjectPath(ast));
			DrawContextMenu(ast, projectPath, AST::NoId);
			UI::EndPopup();
		}

		{
			ZoneScopedN("Draw Files");
			for (auto& item : folders[Name::None()].items)
			{
				DrawItem(ast, item);
			}
		}
		UI::EndChild();
	}

	void FileExplorerPanel::DrawContextMenu(AST::Tree& ast, StringView path, AST::Id itemId)
	{
		const bool hasId    = ast.IsValid(itemId);
		const bool isType   = hasId && ast.Has<AST::CType>(itemId);
		const bool isModule = hasId && ast.Has<AST::CModule>(itemId);

		if (hasId)
		{
			if (isType)
			{
				if (UI::MenuItem("Open"))
				{
					OpenType(ast, itemId);
				}
			}
			else if (isModule)
			{
				if (UI::MenuItem("Open Module"))
				{
					OpenModule(ast, itemId);
				}
			}

			if (UI::MenuItem("Rename"))
			{
				renameId     = itemId;
				renameBuffer = p::GetFilename(path);
			}

			if (isType && UI::MenuItem("Delete"))
			{
				AST::RemoveTypes(ast, itemId, true);
			}

			UI::Separator();
		}

		if (!isType)
		{
			if (UI::BeginMenu("Create type"))
			{
				if (UI::MenuItem("Class"))
				{
					CreateType(ast, "Create Class type", AST::RiftType::Class, path);
				}
				if (UI::MenuItem("Struct"))
				{
					CreateType(ast, "Create Struct type", AST::RiftType::Struct, path);
				}
				if (UI::MenuItem("Static"))
				{
					CreateType(ast, "Create Static type", AST::RiftType::Static, path);
				}
				UI::EndMenu();
			}

			if (UI::MenuItem("Create folder"))
			{
				p::CreateFolder(p::ToPath(path) / "NewFolder");
			}
		}

		if (UI::MenuItem("Show in Explorer"))
		{
			PlatformProcess::ShowFolder(path);
		}
	}

	void FileExplorerPanel::InsertItem(TMap<Name, Folder>& folders, const Item& item)
	{
		Name lastName         = item.path;
		StringView parentPath = p::GetParentPath(item.path.ToString());
		Name parentName{parentPath};

		if (Folder* parent = folders.Find(parentName))
		{
			parent->items.Add(item);
			return;
		}
		folders.Insert(parentName, Folder{{item}});

		parentPath = p::GetParentPath(parentPath);
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

			parentPath = p::GetParentPath(parentPath);
		}
	}

	void FileExplorerPanel::CacheProjectFiles(
	    TAccessRef<AST::CProject, AST::CModule, AST::CFileRef, AST::CType> access)
	{
		ZoneScoped;
		dirty = false;

		folders.Clear();

		// Set root folder (not displayed)
		folders.InsertDefaulted(Name::None());

		projectModuleId = AST::GetProjectId(access);

		// Create module folders
		TArray<AST::Id> modules = ecs::ListAll<AST::CModule>(access);
		TMap<AST::Id, p::Path> moduleFolders;
		moduleFolders.Reserve(modules.Size());
		for (AST::Id moduleId : modules)
		{
			auto& file      = access.Get<const AST::CFileRef>(moduleId);
			Path folderPath = file.path.parent_path();
			const Name pathName{ToString(folderPath)};
			folders.InsertDefaulted(pathName);
			moduleFolders.Insert(moduleId, Move(folderPath));
		}

		// Create folders between modules
		for (AST::Id oneId : modules)
		{
			bool insideOther    = false;
			const p::Path& path = moduleFolders[oneId];

			for (AST::Id otherId : modules)
			{
				if (oneId != otherId)
				{
					const p::Path& otherPath = moduleFolders[otherId];

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
				InsertItem(folders, Item{oneId, Name{p::ToString(path)}, true});
			}
			else
			{
				folders[Name::None()].items.Add(Item{oneId, Name{ToString(path)}, true});
			}
		}

		// Create items
		for (AST::Id typeId : ecs::ListAll<AST::CType, AST::CFileRef>(access))
		{
			auto& file = access.Get<const AST::CFileRef>(typeId);
			if (!file.path.empty())
			{
				const String path = p::ToString(file.path);
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
		const StringView fileName = p::GetFilename(path);

		if (Folder* folder = folders.Find(item.path))
		{
			ImGuiTreeNodeFlags flags = 0;
			if (folder->items.IsEmpty())
			{
				flags |= ImGuiTreeNodeFlags_Bullet;
			}

			auto* module = item.id != AST::NoId ? ast.TryGet<AST::CModule>(item.id) : nullptr;
			if (module)
			{
				// TODO: Display module name
				const String text = Strings::Format(ICON_FA_BOX " {}", fileName);

				UI::PushHeaderColor(UI::primaryColor);
				UI::PushStyleCompact();
				const bool isProject = item.id == projectModuleId;
				if (item.id == projectModuleId)    // Is project
				{
					flags |= ImGuiTreeNodeFlags_DefaultOpen;
				}
				bool open = UI::CollapsingHeader(text.data(), flags);
				UI::PopStyleCompact();
				UI::PopHeaderColor();

				if (UI::BeginPopupContextItem())
				{
					DrawContextMenu(ast, path, item.id);
					UI::EndPopup();
				}

				if (UI::IsItemClicked() && UI::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					OpenModule(ast, item.id);
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
			if (fileName == AST::moduleFilename)
			{
				text = Strings::Format(ICON_FA_FILE_ALT " {}", fileName);
			}
			else if (Strings::EndsWith(fileName, ".rf"))
			{
				text = Strings::Format(ICON_FA_FILE_CODE " {}", fileName.data());
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
					renameBuffer = Strings::RemoveFromEnd(fileName, ".rf");
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

				const StringView parsedNewName = Strings::RemoveFromEnd(renameBuffer, ".rf");
				const bool nameIsEmpty         = parsedNewName.empty();
				const ecs::Id sameNameFuncId =
				    AST::FindChildByName(ast, p::ecs::GetParent(ast, item.id), Name{parsedNewName});
				if (nameIsEmpty || (!IsNone(sameNameFuncId) && item.id != sameNameFuncId))
				{
					UI::PushTextColor(LinearColor::Red());
					UI::SetNextWindowPos(UI::GetCursorScreenPos());
					UI::SetTooltip(nameIsEmpty
					                   ? "A type's name can't be empty"
					                   : "This name is in use by another type in this module");
					UI::PopTextColor();
				}
				else if (UI::IsItemDeactivatedAfterEdit())
				{
					ScopedChange(ast, item.id);
					Path destination = p::ToPath(GetParentPath(path)) / renameBuffer;
					destination.replace_extension("rf");
					// TODO: Move this into systems. Renaming a type shouldnt require so many
					// manual steps
					if (files::Rename(p::ToPath(path), destination))
					{
						if (auto* file = ast.TryGet<AST::CFileRef>(item.id))
						{
							file->path = destination;
						}

						auto& types = ast.GetOrSetStatic<AST::STypes>();
						types.typesByPath.Remove(item.path);
						types.typesByPath.Insert(Name{files::ToString(destination)}, item.id);

						ast.Add<AST::CNamespace>(item.id, Name{parsedNewName});
					}

					renameId         = AST::NoId;
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
					OpenType(ast, item.id);
				}
			}

			if (IsTypeOpen(ast, item.id))
			{
				UI::SameLine(ImGui::GetContentRegionAvailWidth(), 0);
				UI::Bullet();
				UI::NewLine();
			}
		}
	}

	void FileExplorerPanel::DrawModuleActions(AST::Id id, AST::CModule& module) {}
	void FileExplorerPanel::DrawTypeActions(AST::Id id, AST::CType& type) {}

	void FileExplorerPanel::CreateType(
	    AST::Tree& ast, StringView title, AST::RiftType category, p::Path folderPath)
	{
		const p::Path path = files::SaveFileDialog(title, folderPath,
		    {
		        {"Rift Type", Strings::Format("*.{}", Paths::typeExtension)}
        },
		    true);

		AST::Id typeId = AST::CreateType(ast, category, Name::None(), path);

		String data;
		AST::SerializeType(ast, typeId, data);
		files::SaveStringFile(path, data);

		// Destroy the temporal type after saving it
		ast.Destroy(typeId);

		// Mark path to be opened later once the type has loaded
		pendingOpenCreatedPath = path;
	}
}    // namespace rift::Editor
