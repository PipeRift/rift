// Copyright 2015-2022 Piperift - All rights reserved

#include "AST/Utils/ModuleUtils.h"

#include "AST/Serialization.h"
#include "AST/Statics/SModules.h"
#include "AST/Statics/STypes.h"
#include "AST/Systems/FunctionsSystem.h"
#include "AST/Systems/LoadSystem.h"
#include "AST/Systems/TypeSystem.h"
#include "AST/Utils/Paths.h"

#include <ECS/Filtering.h>
#include <Files/Files.h>
#include <Files/Paths.h>
#include <Serialization/Formats/JsonFormat.h>


namespace Rift::Modules
{
	bool ValidateProjectPath(Path& path, String& error)
	{
		if (path.empty())
		{
			error = "Path is empty";
			return false;
		}

		if (Files::IsFile(path))
		{
			if (path.filename() != moduleFile)
			{
				error = "Path is not a rift module file or a folder";
				return false;
			}
			path = ToAbsolutePath(path).parent_path();
		}
		else
		{
			path = ToAbsolutePath(path);
		}
		return true;
	}

	bool CreateProject(Tree& ast, Pipe::Path path)
	{
		String error;
		if (!ValidateProjectPath(path, error))
		{
			Log::Error("Can't create project: {}", error);
			return false;
		}

		if (!Files::ExistsAsFolder(path))
		{
			Files::CreateFolder(path, true);
		}

		const Pipe::Path filePath = path / moduleFile;
		if (Files::ExistsAsFile(filePath))
		{
			Log::Error("Can't create project: Folder already contains a '{}' file", moduleFile);
			return false;
		}

		Serl::JsonFormatWriter writer{};
		writer.GetContext().BeginObject();
		Files::SaveStringFile(filePath, writer.ToString());

		return OpenProject(ast, path);
	}

	bool OpenProject(Tree& ast, Pipe::Path path)
	{
		String error;
		if (!ValidateProjectPath(path, error))
		{
			Log::Error("Can't open project: {}", error);
			return false;
		}

		if (!Files::ExistsAsFolder(path))
		{
			Log::Error("Can't open project: Folder doesn't exist");
			return false;
		}

		const Pipe::Path filePath = path / moduleFile;
		if (!Files::ExistsAsFile(filePath))
		{
			Log::Error("Can't open project: Folder doesn't contain a '{}' file", moduleFile);
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
		return ECS::GetFirst<CProject>(access);
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
			const String fileName = Pipe::ToString(file->path);
			return {GetFilename(GetParentPath(fileName))};    // Folder name
		}
		return {};
	}

	Path GetModulePath(TAccessRef<CFileRef> access, Id moduleId)
	{
		if (const auto* file = access.TryGet<const CFileRef>(moduleId))
		{
			return file->path.parent_path();
		}
		return Pipe::Path{};
	}

	void Serialize(AST::Tree& ast, AST::Id id, String& data)
	{
		ZoneScoped;

		Serl::JsonFormatWriter writer{};
		AST::WriteContext ct{writer.GetContext(), ast, true};
		ct.BeginObject();
		Serl::CommonContext common{ct};
		if (auto* ident = ast.TryGet<CIdentifier>(id))
		{
			ident->SerializeReflection(common);
		}
		if (auto* module = ast.TryGet<CModule>(id))
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
