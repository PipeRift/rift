// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "AST/Id.h"
#include "Pipe/Core/Broadcast.h"

#include <Pipe/Core/Tag.h>
#include <Pipe/Memory/UniquePtr.h>
#include <PipeECS.h>


namespace rift::ast
{
	struct NativeTypeIds
	{
		ast::Id voidId   = ast::NoId;
		ast::Id boolId   = ast::NoId;
		ast::Id floatId  = ast::NoId;
		ast::Id doubleId = ast::NoId;
		ast::Id u8Id     = ast::NoId;
		ast::Id i8Id     = ast::NoId;
		ast::Id u16Id    = ast::NoId;
		ast::Id i16Id    = ast::NoId;
		ast::Id u32Id    = ast::NoId;
		ast::Id i32Id    = ast::NoId;
		ast::Id u64Id    = ast::NoId;
		ast::Id i64Id    = ast::NoId;
		ast::Id stringId = ast::NoId;
	};


	struct Tree : public p::EntityContext
	{
	private:
		static p::TBroadcast<Tree&> onInit;

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

		static const p::TBroadcast<Tree&>& OnInit();

		p::String DumpPools();

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
}    // namespace rift::ast
