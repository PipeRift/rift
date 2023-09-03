// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include <Pipe/Reflect/Struct.h>


namespace rift::AST
{
	struct CDeclaration : public p::Struct
	{
		P_STRUCT(CDeclaration, p::Struct)
	};

}    // namespace rift::AST

namespace std
{
	template<p::Derived<rift::AST::CDeclaration> T>
	struct is_empty<T>
	{
		static constexpr bool value = false;
	};
}    // namespace std
