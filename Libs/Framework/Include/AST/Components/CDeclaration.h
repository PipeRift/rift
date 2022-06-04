// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include <Reflection/Struct.h>


namespace Rift
{
	struct CDeclaration : public Pipe::Struct
	{
		STRUCT(CDeclaration, Pipe::Struct)
	};

}    // namespace Rift

namespace std
{
	template<Pipe::Derived<Rift::CDeclaration> T>
	struct is_empty<T>
	{
		static constexpr bool value = false;
	};
}    // namespace std
