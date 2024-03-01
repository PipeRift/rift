// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Components/CFileRef.h"
#include "AST/Components/CNamespace.h"
#include "AST/Components/CProject.h"
#include "AST/Tree.h"

#include <Pipe/Core/Tag.h>
#include <Pipe/Memory/OwnPtr.h>
#include <PipeECS.h>


namespace rift::ast
{
	struct CModule;
}

namespace rift::ast
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

	bool CreateProject(Tree& ast, p::StringView path);
	bool OpenProject(Tree& ast, p::StringView path);
	void CloseProject(Tree& ast);

	Id CreateModule(Tree& ast, p::StringView path);

	Id GetProjectId(p::TAccessRef<CProject> access);

	p::Tag GetProjectName(p::TAccessRef<CProject, CNamespace, CFileRef> access);
	p::StringView GetProjectPath(p::TAccessRef<CFileRef, CProject> access);
	CModule* GetProjectModule(p::TAccessRef<CProject, p::TWrite<CModule>> access);

	bool HasProject(Tree& ast);

	// Resolve a module's name
	p::Tag GetModuleName(p::TAccessRef<CNamespace, CFileRef> access, Id moduleId);

	// Resolve a module's name
	p::StringView GetModulePath(p::TAccessRef<CFileRef> access, Id moduleId);

	void SerializeModule(ast::Tree& ast, ast::Id id, p::String& data);
	void DeserializeModule(ast::Tree& ast, ast::Id id, const p::String& data);
	const p::TBroadcast<p::EntityReader&>& OnReadModulePools();
	const p::TBroadcast<p::EntityWriter&>& OnWriteModulePools();

	void RegisterModuleBinding(ModuleBinding binding);
	void UnregisterModuleBinding(p::Tag bindingId);
	void AddBindingToModule(ast::Tree& ast, ast::Id id, p::Tag bindingId);
	void RemoveBindingFromModule(ast::Tree& ast, ast::Id id, p::Tag bindingId);
	const ModuleBinding* FindModuleBinding(p::Tag id);
	p::TView<const ModuleBinding> GetModuleBindings();


	template<typename... T>
	void RegisterSerializedModulePools()
	{
		auto components = [](auto& rw) {
			rw.template SerializePools<T...>();
		};
		OnReadModulePools().Bind(components);
		OnWriteModulePools().Bind(components);
	}
}    // namespace rift::ast
