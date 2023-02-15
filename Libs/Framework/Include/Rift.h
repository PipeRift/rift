// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Components/CDeclType.h"
#include "AST/Id.h"
#include "AST/Tree.h"
#include "View.h"

#include <Pipe/Core/Span.h>
#include <Pipe/Core/Tag.h>
#include <Pipe/ECS/Access.h>
#include <Pipe/Reflect/Class.h>
#include <Pipe/Reflect/ClassType.h>


namespace rift
{
	struct FileTypeSettings
	{
		p::String displayName;
		p::String category;
		bool hasVariables      = false;
		bool hasFunctions      = false;
		bool hasFunctionBodies = true;
	};

	struct FileTypeDescriptor
	{
		p::Tag id;
		p::StructType* tagType = nullptr;
		FileTypeSettings settings;
		p::TFunction<void(AST::Tree&, AST::Id)> onAddTag;


		bool operator<(const FileTypeDescriptor& other) const
		{
			return id < other.id;
		}
		friend bool operator<(const p::Tag& lhs, const FileTypeDescriptor& rhs)
		{
			return lhs < rhs.id;
		}
		friend bool operator<(const FileTypeDescriptor& lhs, const p::Tag& rhs)
		{
			return lhs.id < rhs;
		}
	};


	void EnableModule(p::ClassType* type);
	void DisableModule(p::ClassType* type);
	p::TPtr<class Module> GetModule(p::ClassType* type);


	template<typename T>
	void EnableModule()
	{
		EnableModule(T::GetStaticType());
	}
	template<typename T>
	void DisableModule()
	{
		DisableModule(T::GetStaticType());
	}
	template<typename T>
	p::TPtr<T> GetModule()
	{
		return GetModule(T::GetStaticType()).template Cast<T>();
	}

	void RegisterView(View view);
	void RegisterFileType(FileTypeDescriptor&& descriptor);

	p::TSpan<const FileTypeDescriptor> GetFileTypes();
	const FileTypeDescriptor* FindFileType(p::Tag typeId);
	const FileTypeDescriptor* FindFileType(p::TAccessRef<AST::CDeclType> access, AST::Id typeId);

	template<typename TagType>
	void RegisterFileType(p::Tag typeId, FileTypeSettings settings)
	{
		RegisterFileType({
		    .id       = typeId,
		    .tagType  = TagType::GetStaticType(),
		    .settings = p::Move(settings),
		    .onAddTag =
		        [](auto& ast, AST::Id id) {
			ast.template Add<TagType>(id);
		    },
		});
	}
}    // namespace rift
