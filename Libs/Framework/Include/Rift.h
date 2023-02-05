// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "AST/Id.h"
#include "AST/Tree.h"
#include "Pipe/Core/Name.h"
#include "View.h"

#include <Pipe/Core/Span.h>
#include <Pipe/Reflect/Class.h>
#include <Pipe/Reflect/ClassType.h>


namespace rift
{
	struct FileTypeDescriptor
	{
		p::Name id;
		p::StructType* tagType = nullptr;
		p::String displayName;

		p::TFunction<void(AST::Tree&, AST::Id)> onAddTag;


		bool operator<(const FileTypeDescriptor& other) const
		{
			return id < other.id;
		}
		friend bool operator<(const p::Name& lhs, const FileTypeDescriptor& rhs)
		{
			return lhs < rhs.id;
		}
		friend bool operator<(const FileTypeDescriptor& lhs, const p::Name& rhs)
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
	const FileTypeDescriptor* FindFileType(p::Name typeId);

	template<typename TagType>
	void RegisterFileType(p::Name typeId, p::StringView displayName = {})
	{
		RegisterFileType(FileTypeDescriptor{.id = typeId,
		    .tagType                            = TagType::GetStaticType(),
		    .displayName                        = p::String{displayName},
		    .onAddTag                           = [](auto& ast, AST::Id id) {
                ast.Add<TagType>(id);
		    }});
	}
}    // namespace rift
