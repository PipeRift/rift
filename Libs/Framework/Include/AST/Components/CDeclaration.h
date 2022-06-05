// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace rift
{
	struct CDeclaration : public pipe::Struct
	{
		STRUCT(CDeclaration, pipe::Struct)
	};

}    // namespace rift

namespace std
{
	template<pipe::Derived<rift::CDeclaration> T>
	struct is_empty<T>
	{
		static constexpr bool value = false;
	};
}    // namespace std
