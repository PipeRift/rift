// Copyright 2015-2020 Piperift - All rights reserved
#pragma once

#include "Memory/Allocator.h"


namespace Rift
{
	namespace Refl
	{
		class Struct;
	}

	struct BaseStruct
	{
		BaseStruct() = default;
		virtual ~BaseStruct() {}

		Refl::Struct* GetType() const;

		void* operator new(size_t size)
		{
			return Memory::GetObjectsAllocator()->Allocate(size);
		}
		void operator delete(void* p, std::size_t size)
		{
			Memory::GetObjectsAllocator()->Deallocate(p, size);
		}
	};
}	 // namespace Rift