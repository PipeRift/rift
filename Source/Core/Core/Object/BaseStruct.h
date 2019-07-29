// Copyright 2015-2019 Piperift - All rights reserved
#pragma once

#include "../Memory/Allocator.h"

struct BaseStruct {
	BaseStruct() = default;
	virtual ~BaseStruct() {}

	class StructType* GetStruct() const;

	void* operator new(size_t size) { return Memory::GetObjectsAllocator()->Allocate(size); }
	void operator delete(void* p, std::size_t size) { Memory::GetObjectsAllocator()->Deallocate(p, size); }
};
