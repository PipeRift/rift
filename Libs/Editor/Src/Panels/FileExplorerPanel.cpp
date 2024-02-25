// Copyright 2015-2023 Piperift - All rights reserved

#include "Panels/FileExplorerPanel.h"

#include "Editor.h"
#include "Pipe/Core/String.h"
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
#include <Pipe/Core/StringView.h>
#include <Pipe/Files/Files.h>
#include <Pipe/Files/Paths.h>
#include <Pipe/Files/PlatformPaths.h>
#include <Pipe/Files/STDFileSystem.h>
#include <PipeFiles.h>
#include <Rift.h>
#include <UI/Style.h>
#include <UI/UI.h>


namespace rift::Editor
{
	void FileExplorerPanel::Draw(ast::Tree& ast)
	{
		// Open recently created types
		if (!pendingOpenCreatedPath.empty())
		{
			ast::Id typeId = ast::FindTypeByPath(ast, pendingOpenCreatedPath);
			if (!IsNone(typeId))
			{
				OpenType(ast, typeId);
				pendingOpenCreatedPath = {};
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

	void FileExplorerPanel::DrawList(ast::Tree& ast)
	{
		// if (dirty)
		{
			CacheProjectFiles(ast);
		}

		UI::BeginChild("Files");
		if (UI::BeginPopupContextWindow())
		{
			String projectPath{ast::GetProjectPath(ast)};
			DrawContextMenu(ast, projectPath, ast::NoId);
			UI::EndPopup();
		}

		{
			for (auto& item : folders[{}].items)
			{
				DrawItem(ast, item);
			}
		}
		UI::EndChild();
	}

	void FileExplorerPanel::DrawContextMenu(ast::Tree& ast, StringView path, ast::Id itemId)
	{
		const bool hasId    = ast.IsValid(itemId);
		const bool isType   = hasId && ast.Has<ast::CDeclType>(itemId);
		const bool isModule = hasId && ast.Has<ast::CModule>(itemId);

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
					OpenModuleEditor(ast, itemId);
				}
			}

			if (UI::MenuItem("Rename"))
			{
				renameId     = itemId;
				renameBuffer = p::GetFilename(path);
			}

			if (isType && UI::MenuItem("Delete"))
			{
				ast::RemoveTypes(ast, itemId, true);
			}

			UI::Separator();
		}

		if (!isType)
		{
			if (UI::BeginMenu("Create type"))
			{
				TArray<const ast::RiftType*> types;
				types.Reserve(ast::GetFileTypes().Size());
				for (const auto& type : ast::GetFileTypes())
				{
					types.Add(&type);
				}
				types.Sort([](const auto* one, const auto* other) {
					return one->settings.category < other->settings.category;
				});

				p::StringView currentCategory;
				String createText;
				for (const auto& fileType : types)
				{
					if (currentCategory != fileType->settings.category)
					{
						currentCategory = fileType->settings.category;
						ImGui::Separator();
						ImGui::MenuItem(currentCategory.data(), nullptr, false, false);
					}

					createText.clear();
					Strings::FormatTo(
					    createText, "{}##{}", fileType->settings.displayName, fileType->id);
					if (UI::MenuItem(createText.c_str()))
					{
						createText.clear();
						Strings::FormatTo(
						    createText, "Create {} type", fileType->settings.displayName);
						CreateType(ast, createText, fileType->id, path);
					}
				}
				UI::EndMenu();
			}

			if (UI::MenuItem("Create folder"))
			{
				p::CreateFolder(p::JoinPaths(path, "NewFolder"));
			}
		}

		if (UI::MenuItem("Show in Explorer"))
		{
			PlatformPaths::ShowFolder(path);
		}
	}

	void FileExplorerPanel::InsertItem(const Item& item)
	{
		StringView parentPath = p::GetParentPath(item.path);
		Tag parentName{parentPath};

		if (Folder* parent = folders.Find(parentName))
		{
			parent->items.Add(item);
			return;
		}
		folders.Insert(parentName, Folder{.items = {item}});

		StringView lastPath;
		while (!parentPath.empty())
		{
			lastPath   = parentPath;
			parentPath = p::GetParentPath(parentPath);
			parentName = p::Tag{parentPath};

			const Item newItem{.id = ast::NoId, .path = p::String{lastPath}, .isFolder = true};
			if (Folder* parent = folders.Find(parentName))
			{
				parent->items.Add(newItem);
				// Found an existing folder. Leave since we assume all parent folders are valid
				return;
			}
			folders.Insert(parentName, Folder{.items = {newItem}});
		}
	}

	void FileExplorerPanel::CacheProjectFiles(
	    TAccessRef<ast::CProject, ast::CModule, ast::CFileRef, ast::CDeclType> access)
	{
		dirty = false;

		folders.Clear();

		// Set root folder (not displayed)
		folders.InsertDefaulted({});

		projectModuleId = ast::GetProjectId(access);

		// Create module folders
		TArray<ast::Id> modules = FindAllIdsWith<ast::CModule>(access);
		TMap<ast::Id, p::StringView> moduleFolders;
		moduleFolders.Reserve(modules.Size());
		for (ast::Id moduleId : modules)
		{
			auto& file               = access.Get<const ast::CFileRef>(moduleId);
			p::StringView folderPath = p::GetParentPath(file.path);
			folders.InsertDefaulted(p::Tag{folderPath});
			moduleFolders.Insert(moduleId, folderPath);
		}

		// Create folders between modules
		for (ast::Id oneId : modules)
		{
			bool insideOther         = false;
			const p::StringView path = moduleFolders[oneId];

			for (ast::Id otherId : modules)
			{
				if (oneId != otherId
				    && Strings::StartsWith(path, moduleFolders[otherId]))    // Is relative
				{
					insideOther = true;
					break;
				}
			}

			if (insideOther)
			{
				// If a module is inside another, create the folders in between
				InsertItem(Item{.id = oneId, .path = p::String{path}, .isFolder = true});
			}
			else
			{
				// Add at root folder
				folders[{}].items.Add(Item{.id = oneId, .path = p::String{path}, .isFolder = true});
			}
		}

		// Create items
		for (ast::Id typeId : FindAllIdsWith<ast::CDeclType, ast::CFileRef>(access))
		{
			auto& file = access.Get<const ast::CFileRef>(typeId);
			if (!file.path.empty())
			{
				InsertItem(Item{typeId, file.path});
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
			return one.path < other.path;
		});
	}

	void FileExplorerPanel::DrawItem(ast::Tree& ast, const Item& item)
	{
		const String path         = p::ToString(item.path);
		const StringView fileName = p::GetFilename(path);

		const StringView dirty =
		    (item.id != ast::NoId && ast.Has<ast::CFileDirty>(item.id)) ? " *" : "";

		if (Folder* folder = folders.Find(p::Tag{item.path}))
		{
			ImGuiTreeNodeFlags flags = 0;
			auto* module = item.id != ast::NoId ? ast.TryGet<ast::CModule>(item.id) : nullptr;
			if (module)
			{
				// TODO: Display module name
				const String text = Strings::Format(ICON_FA_TH_LARGE " {}{}", fileName, dirty);

				UI::PushHeaderColor(UI::GetNeutralColor(1));
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
					OpenModuleEditor(ast, item.id);
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
			if (fileName == ast::moduleFilename)
			{
				text = Strings::Format(ICON_FA_TH_LARGE " {}{}", fileName, dirty);
			}
			else if (Strings::EndsWith(fileName, ".rf"))
			{
				StringView icon;
				if (ast::IsStructType(ast, item.id))
					icon = ICON_FA_FILE_ALT;
				else if (ast::IsClassType(ast, item.id))
					icon = ICON_FA_FILE_INVOICE;
				else if (ast::IsStaticType(ast, item.id))
					icon = ICON_FA_FILE_WORD;

				text = Strings::Format("{} {}{}", icon, fileName, dirty);
			}
			else
			{
				text = fileName;
			}

			if (renameId != item.id)
			{
				UI::TreeNodeEx(
				    text.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
				if (UI::IsItemHovered() && UI::IsKeyReleased(ImGuiKey_F2))
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

				const StringView parsedNewName =
				    Strings::RemoveFromEnd(p::StringView{renameBuffer}, ".rf");
				const bool nameIsEmpty = parsedNewName.empty();
				const Id sameNameFuncId =
				    ast::FindChildByName(ast, p::GetIdParent(ast, item.id), Tag{parsedNewName});
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
					p::String destination = p::JoinPaths(p::GetParentPath(path), renameBuffer);
					p::ReplaceExtension(destination, "rf");
					// TODO: Move this into systems. Renaming a type shouldnt require so many
					// manual steps
					if (files::Rename(path, destination))
					{
						if (auto* file = ast.TryGet<ast::CFileRef>(item.id))
						{
							file->path = destination;
						}

						auto& types = ast.GetOrSetStatic<ast::STypes>();
						types.typesByPath.Remove(p::Tag{item.path});
						types.typesByPath.Insert(p::Tag{destination}, item.id);

						ast.Add<ast::CNamespace>(item.id, Tag{parsedNewName});
					}

					renameId         = ast::NoId;
					renameBuffer     = "";
					renameHasFocused = false;
				}
				else if (UI::IsItemDeactivated())
				{
					renameId         = ast::NoId;
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
				UI::SameLine(ImGui::GetContentRegionAvail().x, 0);
				UI::Bullet();
				UI::NewLine();
			}
		}
	}

	void FileExplorerPanel::DrawModuleActions(ast::Id id, ast::CModule& module) {}
	void FileExplorerPanel::DrawTypeActions(ast::Id id, ast::CDeclType& type) {}

	void FileExplorerPanel::CreateType(
	    ast::Tree& ast, StringView title, p::Tag typeId, p::StringView folderPath)
	{
		const p::String path = p::SaveFileDialog(title, folderPath,
		    {
		        {"Rift Type", Strings::Format("*.{}", Paths::typeExtension)}
        },
		    true);

		ast::Id id = ast::CreateType(ast, typeId, Tag::None(), path);

		String data;
		ast::SerializeType(ast, id, data);
		files::SaveStringFile(StringView{path}, data);

		// Destroy the temporal type after saving it
		ast.Destroy(id);

		// Mark path to be opened later once the type has loaded
		pendingOpenCreatedPath = path;
	}
}    // namespace rift::Editor
