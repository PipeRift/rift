// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/CModule.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/CProject.h"
#include "AST/Tree.h"

#include <Pipe/ECS/Filtering.h>
#include <Pipe/Memory/OwnPtr.h>


namespace rift::AST
{
	struct CModule;
}

namespace rift::AST
{
	struct ModuleBinding
	{
		p::Tag id;
		p::StructType* tagType = nullptr;
		p::String displayName;

		bool operator<(const ModuleBinding& other) const
		{
			return id < other.id;
		}
		friend bool operator<(const p::Tag& lhs, const ModuleBinding& rhs)
		{
			return lhs < rhs.id;
		}
		friend bool operator<(const ModuleBinding& lhs, const p::Tag& rhs)
		{
			return lhs.id < rhs;
		}
	};


	using namespace p::core;
	using namespace p::files;

	bool CreateProject(Tree& ast, StringView path);
	bool OpenProject(Tree& ast, StringView path);
	void CloseProject(Tree& ast);

	Id CreateModule(Tree& ast, StringView path);

	Id GetProjectId(TAccessRef<CProject> access);

	Tag GetProjectName(TAccessRef<CProject, CNamespace, CFileRef> access);
	p::StringView GetProjectPath(TAccessRef<CFileRef, CProject> access);
	CModule* GetProjectModule(TAccessRef<CProject, TWrite<CModule>> access);

	bool HasProject(Tree& ast);

	// Resolve a module's name
	Tag GetModuleName(TAccessRef<CNamespace, CFileRef> access, Id moduleId);

	// Resolve a module's name
	p::StringView GetModulePath(TAccessRef<CFileRef> access, Id moduleId);

	void SerializeModule(AST::Tree& ast, AST::Id id, String& data);
	void DeserializeModule(AST::Tree& ast, AST::Id id, const String& data);

	void RegisterModuleBinding(ModuleBinding binding);
	void UnregisterModuleBinding(p::Tag bindingId);
	void AddBindingToModule(AST::Tree& ast, AST::Id id, p::Tag bindingId);
	void RemoveBindingFromModule(AST::Tree& ast, AST::Id id, p::Tag bindingId);
	const ModuleBinding* FindModuleBinding(p::Tag id);
	p::TSpan<const ModuleBinding> GetModuleBindings();
}    // namespace rift::AST
