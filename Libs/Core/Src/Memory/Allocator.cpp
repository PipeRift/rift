// Copyright 2015-2020 Piperift - All rights reserved

#include "Memory/Allocator.h"
#include "Memory/NewDelete.h"
#include "Strings/Name.h"

#include <Tracy.hpp>


namespace Rift
{
	void* Alloc(size_t n)
	{
		void* const p = mi_malloc(n);
		TracyAllocS(p, n, 8);
		return p;
	}

	void Free(void* p)
	{
		TracyFreeS(p, 8);
		mi_free(p);
	}

	namespace Memory
	{
		/// Default Arena instance.
		Arena gArena{TX("Global")};
		Arena* GetGlobalArena()
		{
			return &gArena;
		}

		Arena gAssetsArena{TX("Assets")};
		Arena* GetAssetsArena()
		{
			return &gAssetsArena;
		}
	};	  // namespace Memory
}	 // namespace Rift
