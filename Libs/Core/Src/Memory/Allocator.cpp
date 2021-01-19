// Copyright 2015-2020 Piperift - All rights reserved

#include "Memory/Allocator.h"
#include "Strings/Name.h"

#include <Tracy.hpp>


namespace Rift
{
	Allocator::Allocator(const TCHAR* name /*= "Global"*/) : name{name}, size{0}, malloc_alloc{""}
	{}

	Allocator::Allocator(const Allocator&) : name{}, size{0}, malloc_alloc{} {}

	Allocator::Allocator(const Allocator&, const TCHAR* name)
		: name{name}
		, size{0}
		, malloc_alloc{""}
	{}

	Name Allocator::GetName() const
	{
		return {name};
	}

	void Allocator::SetName(const TCHAR* newName)
	{
		name = newName;
	}


	namespace Memory
	{
		/// gAllocator
		/// Default engine allocator instance.
		CORE_API Allocator gAllocator{};
		CORE_API Allocator* GetAllocator()
		{
			return &gAllocator;
		}

		/// gAllocator
		/// Default engine allocator instance.
		CORE_API Allocator gObjectsAllocator{TX("Objects")};
		CORE_API Allocator* GetObjectsAllocator()
		{
			return &gObjectsAllocator;
		}

		CORE_API Allocator gAssetsAllocator{TX("Assets")};
		CORE_API Allocator* GetAssetsAllocator()
		{
			return &gAssetsAllocator;
		}

		CORE_API Allocator gFrameAllocator{TX("Frame")};
		CORE_API Allocator* GetFrameAllocator()
		{
			return &gFrameAllocator;
		}
	};	  // namespace Memory
}	 // namespace Rift

#if defined(__cplusplus)
#	include <mimalloc.h>
#	include <new>

void operator delete(void* p) noexcept
{
	TracyFreeS(p, 8);
	mi_free(p);
};
void operator delete[](void* p) noexcept
{
	TracyFreeS(p, 8);
	mi_free(p);
};

void* operator new(std::size_t n) noexcept(false)
{
	void* const p = mi_new(n);
	TracyAllocS(p, n, 8);
	return p;
}
void* operator new[](std::size_t n) noexcept(false)
{
	void* const p = mi_new(n);
	TracyAllocS(p, n, 8);
	return p;
}

void* operator new(std::size_t n, const std::nothrow_t& tag) noexcept
{
	(void) (tag);
	void* const p = mi_new_nothrow(n);
	TracyAllocS(p, n, 8);
	return p;
}
void* operator new[](std::size_t n, const std::nothrow_t& tag) noexcept
{
	(void) (tag);
	void* const p = mi_new_nothrow(n);
	TracyAllocS(p, n, 8);
	return p;
}

#	if (__cplusplus >= 201402L || _MSC_VER >= 1916)
void operator delete(void* p, std::size_t n) noexcept
{
	TracyFreeS(p, 8);
	mi_free_size(p, n);
};
void operator delete[](void* p, std::size_t n) noexcept
{
	TracyFreeS(p, 8);
	mi_free_size(p, n);
};
#	endif

#	if (__cplusplus > 201402L || defined(__cpp_aligned_new))
void operator delete(void* p, std::align_val_t al) noexcept
{
	TracyFreeS(p, 8);
	mi_free_aligned(p, static_cast<size_t>(al));
}
void operator delete[](void* p, std::align_val_t al) noexcept
{
	TracyFreeS(p, 8);
	mi_free_aligned(p, static_cast<size_t>(al));
}
void operator delete(void* p, std::size_t n, std::align_val_t al) noexcept
{
	TracyFreeS(p, 8);
	mi_free_size_aligned(p, n, static_cast<size_t>(al));
};
void operator delete[](void* p, std::size_t n, std::align_val_t al) noexcept
{
	TracyFreeS(p, 8);
	mi_free_size_aligned(p, n, static_cast<size_t>(al));
};

void* operator new(std::size_t n, std::align_val_t al) noexcept(false)
{
	void* const p = mi_new_aligned(n, static_cast<size_t>(al));
	TracyAllocS(p, n, 8);
	return p;
}
void* operator new[](std::size_t n, std::align_val_t al) noexcept(false)
{
	void* const p = mi_new_aligned(n, static_cast<size_t>(al));
	TracyAllocS(p, n, 8);
	return p;
}
void* operator new(std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept
{
	void* const p = mi_new_aligned_nothrow(n, static_cast<size_t>(al));
	TracyAllocS(p, n, 8);
	return p;
}
void* operator new[](std::size_t n, std::align_val_t al, const std::nothrow_t&) noexcept
{
	void* const p = mi_new_aligned_nothrow(n, static_cast<size_t>(al));
	TracyAllocS(p, n, 8);
	return p;
}
#	endif
#endif