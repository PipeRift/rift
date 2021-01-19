// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "CoreEngine.h"

#include <EASTL/allocator_malloc.h>
#include <mimalloc.h>

#include <Tracy.hpp>
#include <client/tracy_rpmalloc.hpp>
#include <new>


namespace Rift
{
	///////////////////////////////////////////////////////////////////////////////
	// Allocator
	//
	class Allocator
	{
		const TCHAR* name;
		size_t size;
		eastl::allocator_malloc malloc_alloc;

	public:
		Allocator(const TCHAR* name = TX("Global"));
		Allocator(const Allocator&);
		Allocator(const Allocator&, const TCHAR* name);

		Allocator& operator=(const Allocator&)
		{
			return *this;
		}

		bool operator==(const Allocator&)
		{
			return true;
		}

		bool operator!=(const Allocator&)
		{
			return false;
		}

		void* Allocate(size_t n, int flags = 0)
		{
			// size += n;
			void* const ptr = malloc_alloc.allocate(n, flags);
			TracyAllocS(ptr, n, 8);
			return ptr;
		}

		void* Allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags = 0)
		{
			// size += n;
			void* const ptr = malloc_alloc.allocate(n, alignment, alignmentOffset, flags);
			TracyAllocS(ptr, n, 8);
			return ptr;
		}

		void Deallocate(void* const ptr)
		{
			// #TODO: obtain size of the ptr
			TracyFreeS(ptr, 8);
			malloc_alloc.deallocate(ptr, 0);
		}

		void Deallocate(void* const ptr, size_t n)
		{
			// size -= n;
			TracyFreeS(ptr, 8);
			malloc_alloc.deallocate(ptr, n);
		}

		struct Name GetName() const;

		void SetName(const TCHAR* Name);

		size_t GetSize() const
		{
			return size;
		}
	};

	namespace Memory
	{
		Allocator* GetAllocator();

		Allocator* GetObjectsAllocator();

		Allocator* GetAssetsAllocator();

		Allocator* GetFrameAllocator();
	}	 // namespace Memory
}	 // namespace Rift
