// Copyright 2015-2020 Piperift - All rights reserved

#pragma once

#include "Platform/Platform.h"

#include <robin_hood.h>


namespace VCLang
{
	template <typename T>
	struct Hash : robin_hood::hash<T>
	{
		size_t operator()(T const& obj)
		{
			return robin_hood::hash<T>::operator()(obj);
		}
	};

	inline sizet HashBytes(void const* ptr, sizet const len) noexcept
	{
		return robin_hood::hash_bytes(ptr, len);
	}
}	 // namespace VCLang
