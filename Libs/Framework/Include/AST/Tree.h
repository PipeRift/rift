// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"
#include "Pipe/Core/Broadcast.h"

#include <Pipe/Core/Tag.h>
#include <Pipe/ECS/Context.h>
#include <Pipe/Memory/UniquePtr.h>


namespace rift::AST
{
	struct NativeTypeIds
	{
		AST::Id voidId   = AST::NoId;
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


	struct Tree : public p::ecs::Context
	{
	private:
		static TBroadcast<Tree&> onInit;

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

		static const TBroadcast<Tree&>& OnInit();

	private:
		void CopyFrom(const Tree& other);
		void MoveFrom(Tree&& other);

		void SetupNativeTypes();
	};


	template<typename... T>
	void PreAllocPools()
	{
		Tree::OnInit().Bind([](const Tree& ast) {
			(ast.AssurePool<T>(), ...);
		});
	}
}    // namespace rift::AST
