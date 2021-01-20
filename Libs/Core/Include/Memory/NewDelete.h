#pragma once

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
