// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include <Types/Struct.h>


namespace Rift
{
	struct CDeclaration : public Struct
	{
		STRUCT(CDeclaration, Struct)
	};

}    // namespace Rift

namespace std
{
	template<Rift::Derived<Rift::CDeclaration> T>
	struct is_empty<T>
	{
		static constexpr bool value = false;
	};
}    // namespace std
