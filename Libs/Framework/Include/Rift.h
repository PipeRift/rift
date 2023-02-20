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
	struct RiftTypeSettings
	{
		p::String displayName;
		p::String category;
		bool hasVariables      = false;
		bool hasFunctions      = false;
		bool hasFunctionBodies = true;
	};

	struct RiftTypeDescriptor
	{
		p::Tag id;
		p::StructType* tagType = nullptr;
		RiftTypeSettings settings;

		bool operator<(const RiftTypeDescriptor& other) const
		{
			return id < other.id;
		}
		friend bool operator<(const p::Tag& lhs, const RiftTypeDescriptor& rhs)
		{
			return lhs < rhs.id;
		}
		friend bool operator<(const RiftTypeDescriptor& lhs, const p::Tag& rhs)
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
	void RegisterRiftType(RiftTypeDescriptor&& descriptor);
	void UnregisterRiftType(p::Tag typeId);

	p::TSpan<const RiftTypeDescriptor> GetRiftTypes();
	const RiftTypeDescriptor* FindRiftType(p::Tag typeId);
	const RiftTypeDescriptor* FindRiftType(p::TAccessRef<AST::CDeclType> access, AST::Id typeId);

	template<typename TagType>
	void RegisterRiftType(p::Tag typeId, RiftTypeSettings settings)
	{
		RegisterRiftType(
		    {.id = typeId, .tagType = TagType::GetStaticType(), .settings = p::Move(settings)});
	}
}    // namespace rift
