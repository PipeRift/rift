// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"

#include <Core/Name.h>
#include <ECS/Context.h>
#include <Memory/UniquePtr.h>


namespace Rift::AST
{
	struct NativeTypeIds
	{
		AST::Id boolId   = AST::NoId;
		AST::Id floatId  = AST::NoId;
		AST::Id doubleId = AST::NoId;
		AST::Id u8Id     = AST::NoId;
		AST::Id i8Id     = AST::NoId;
		AST::Id u16Id    = AST::NoId;
		AST::Id i16Id    = AST::NoId;
		AST::Id u32Id    = AST::NoId;
		AST::Id i32Id    = AST::NoId;
		AST::Id u64Id    = AST::NoId;
		AST::Id i64Id    = AST::NoId;
		AST::Id stringId = AST::NoId;
	};

	struct Tree : public Pipe::ECS::Context
	{
	private:
		NativeTypeIds nativeTypes;


	public:
		Tree();
		explicit Tree(const Tree& other) noexcept;
		explicit Tree(Tree&& other) noexcept;
		Tree& operator=(const Tree& other) noexcept;
		Tree& operator=(Tree&& other) noexcept;

		const NativeTypeIds& GetNativeTypes() const
		{
			return nativeTypes;
		}


	private:
		void CopyFrom(const Tree& other);
		void MoveFrom(Tree&& other);

		void SetupNativeTypes();
	};
}    // namespace Rift::AST
