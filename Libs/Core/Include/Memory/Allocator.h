// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"

#include <mimalloc.h>

#include <Tracy.hpp>
#include <client/tracy_rpmalloc.hpp>
#include <new>


namespace Rift
{
	void* Alloc(size_t n);

	void Free(void* p);

	namespace Memory
	{
		class CORE_API Arena
		{
			const char* name;

		public:
			Arena(const TCHAR* name) : name(name) {}

			const char* GetName() const
			{
				return name;
			}
		};

		CORE_API Arena* GetGlobalArena();
		CORE_API Arena* GetAssetsArena();
	}	 // namespace Memory
}	 // namespace Rift
