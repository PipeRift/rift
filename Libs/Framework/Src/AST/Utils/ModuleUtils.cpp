// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"

#include "AST/Serialization.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"
#include "AST/Systems/FunctionsSystem.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "Framework/Paths.h"

#include <Files/Files.h>
#include <Files/Paths.h>
#include <Serialization/Formats/JsonFormat.h>


namespace Rift::Modules
{
	bool OpenProject(Tree& ast, const Path& path)
	{
		if (path.empty())
		{
			Log::Error("Can't open project: Invalid path");
			return false;
		}

		Path folderPath;
		Path filePath;
		if (Files::IsFile(path))
		{
			filePath   = Paths::ToAbsolute(path);
			folderPath = filePath.parent_path();
		}
		else
		{
			folderPath = Paths::ToAbsolute(path);
			filePath   = folderPath / moduleFile;
		}

		if (!Files::ExistsAsFile(filePath))
		{
			Log::Error(
			    "Can't open project: Project file failed to load. Does it exist? Is it corrupted?");
			return false;
		}

		ast = Tree{};
		ast.SetStatic<SModules>();
		ast.SetStatic<STypes>();
		LoadSystem::Init(ast);
		TypeSystem::Init(ast);
		FunctionsSystem::Init(ast);

		// Create project node (root module)
		Id projectId = ast.Create();
		ast.Add<CProject, CModule>(projectId);
		ast.Add<CFileRef>(projectId, filePath);

		// Load project module
		TArray<String> strings;
		LoadSystem::LoadFileStrings(ast, projectId, strings);
		LoadSystem::DeserializeModules(ast, projectId, strings);
		return true;
	}

	void CloseProject(Tree& ast)
	{
		ast.Reset();
	}

	Id GetProjectId(TAccessRef<CProject> access)
	{
		return GetFirst<CProject>(access);
	}

	Name GetProjectName(TAccessRef<CProject, CIdentifier, CFileRef> access)
	{
		Id moduleId = GetProjectId(access);
		return GetModuleName(access, moduleId);
	}

	Path GetProjectPath(TAccessRef<CFileRef, CProject> access)
	{
		return GetModulePath(access, GetProjectId(access));
	}

	CModule* GetProjectModule(TAccessRef<CProject, TWrite<CModule>> access)
	{
		const Id projectId = GetProjectId(access);
		if (projectId != NoId)
		{
			return access.TryGet<CModule>(projectId);
		}
		return nullptr;
	}

	bool HasProject(Tree& ast)
	{
		return GetProjectId(ast) != NoId;
	}

	Name GetModuleName(TAccessRef<CIdentifier, CFileRef> access, Id moduleId)
	{
		if (!access.IsValid(moduleId))
		{
			return {};
		}

		const auto* identifier = access.TryGet<const CIdentifier>(moduleId);
		if (identifier && !identifier->name.IsNone())
		{
			return identifier->name;
		}

		const auto* file = access.TryGet<const CFileRef>(moduleId);
		if (file && !file->path.empty())
		{
			// Obtain name from project file name
			const String fileName = Paths::ToString(file->path);
			return {Paths::GetFilename(Paths::GetParent(fileName))};    // Folder name
		}
		return {};
	}

	Path GetModulePath(TAccessRef<CFileRef> access, Id moduleId)
	{
		if (const auto* file = access.TryGet<const CFileRef>(moduleId))
		{
			return file->path.parent_path();
		}
		return Path{};
	}

	void Serialize(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;

		Serl::JsonFormatWriter writer{};
		AST::WriteContext ct{writer.GetContext(), ast, true};
		ct.BeginObject();
		Serl::CommonContext common{ct};
		if (CIdentifier* ident = ast.TryGet<CIdentifier>(id))
		{
			ident->SerializeReflection(common);
		}
		if (CModule* module = ast.TryGet<CModule>(id))
		{
			module->SerializeReflection(common);
		}
		data = writer.ToString();
	}

	void Deserialize(AST::Tree& ast, AST::Id id, const String& data)
	{
		ZoneScoped;

		Serl::JsonFormatReader reader{data};
		if (!reader.IsValid())
		{
			return;
		}

		AST::ReadContext ct{reader, ast};
		ct.BeginObject();
		Serl::CommonContext common{ct};
		ast.GetOrAdd<CIdentifier>(id).SerializeReflection(common);
		ast.GetOrAdd<CModule>(id).SerializeReflection(common);
	}
}    // namespace Rift::Modules
