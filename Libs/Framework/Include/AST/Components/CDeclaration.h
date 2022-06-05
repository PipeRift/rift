// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace rift
{
	struct CDeclaration : public p::Struct
	{
		STRUCT(CDeclaration, p::Struct)
	};

}    // namespace rift

namespace std
{
	template<p::Derived<rift::CDeclaration> T>
	struct is_empty<T>
	{
		static constexpr bool value = false;
	};
}    // namespace std
