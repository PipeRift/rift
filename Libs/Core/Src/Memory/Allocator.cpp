// Copyright 2015-2020 Piperift - All rights reserved

#include "Memory/Allocator.h"

#include "Strings/Name.h"


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
		EASTL_API Allocator gAllocator{};
		EASTL_API Allocator* GetAllocator()
		{
			return &gAllocator;
		}

		/// gAllocator
		/// Default engine allocator instance.
		EASTL_API Allocator gObjectsAllocator{TX("Objects")};
		EASTL_API Allocator* GetObjectsAllocator()
		{
			return &gObjectsAllocator;
		}

		EASTL_API Allocator gAssetsAllocator{TX("Assets")};
		EASTL_API Allocator* GetAssetsAllocator()
		{
			return &gAssetsAllocator;
		}

		EASTL_API Allocator gFrameAllocator{TX("Frame")};
		EASTL_API Allocator* GetFrameAllocator()
		{
			return &gFrameAllocator;
		}
	};	  // namespace Memory
}	 // namespace Rift


// EASTL News
void* operator new[](size_t size, const char* /*name*/, int flags, unsigned /*debugFlags*/,
	const char* /*file*/, int /*line*/)
{
	return Rift::Memory::GetAllocator()->Allocate(size, flags);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* /*name*/,
	int flags, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
	return Rift::Memory::GetAllocator()->Allocate(size, alignment, alignmentOffset, flags);
}


// Native News
void* operator new(size_t size)
{
	return Rift::Memory::GetAllocator()->Allocate(size);
}

void* operator new[](size_t size)
{
	return Rift::Memory::GetAllocator()->Allocate(size);
}


// Deletes
void operator delete(void* p, std::size_t size)
{
	Rift::Memory::GetAllocator()->Deallocate(p, size);
}
void operator delete(void* p)
{
	Rift::Memory::GetAllocator()->Deallocate(p);
}

void operator delete[](void* p, std::size_t size)
{
	Rift::Memory::GetAllocator()->Deallocate(p, size);
}
void operator delete[](void* p)
{
	Rift::Memory::GetAllocator()->Deallocate(p);
}