#ifndef __ALLOC_H__
#define __ALLOC_H__

#pragma once

using static_pool_t = memory::memory_pool<memory::node_pool, memory::static_block_allocator>;
using stack_string_t = memory::string<static_pool_t>;

#ifndef USE_ZONE
void Mem_Init(void);
void Mem_Shutdown(void);
void *Mem_Alloc(const uint32_t size);
void Mem_Free(void *ptr);
#endif

template<typename T>
struct heap_allocator_template
{
    heap_allocator_template(const char* _name = "allocator") noexcept { }
    template<typename U>
	heap_allocator_template(const heap_allocator_template<U> &) noexcept { }

    typedef T value_type;

    inline T* allocate(size_t n) const
    { return static_cast<T *>(GetMemory(n)); }
	inline T* allocate(size_t& n, size_t& alignment, size_t& offset) const
    { return static_cast<T *>(GetMemory(n)); }
	inline T* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags) const
    { return static_cast<T *>(GetMemory(n)); }
	inline void deallocate(void *p, size_t) const noexcept
    { FreeMemory(p); }
};

template<typename T>
struct heap_allocator_template
{
    heap_allocator_template(const char* _name = "allocator") noexcept { }
    template<typename U>
	heap_allocator_template(const heap_allocator_template<U> &) noexcept { }

    typedef T value_type;

    inline T* allocate(size_t n) const
    {
    #ifdef USE_ZONE
        return static_cast<T *>(Z_Malloc(n, TAG_STATIC, NULL, "allocator"));
    #else
        return static_cast<T *>(Mem_Alloc(n));
    #endif
    }
	inline T* allocate(size_t& n, size_t& alignment, size_t& offset) const
    {
    #ifdef USE_ZONE
        return static_cast<T *>(Z_Malloc(n, TAG_STATIC, NULL, "allocator"));
    #else
        return static_cast<T *>(Mem_Alloc(n));
    #endif
    }
	inline T* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags) const
    {
    #ifdef USE_ZONE
        return static_cast<T *>(Z_Malloc(n, TAG_STATIC, NULL, "allocator"));
    #else
        return static_cast<T *>(Mem_Alloc(n));
    #endif
    }
	inline void deallocate(void *p, size_t) const noexcept
    {
    #ifdef USE_ZONE
        Z_Free(p);
    #else
        Mem_Free(p);
    #endif
    }
};

struct heap_allocator
{
	constexpr heap_allocator(void) noexcept { }
	constexpr heap_allocator(const char* _name = "allocator") noexcept { }
	constexpr heap_allocator(const heap_allocator &) noexcept { }

	inline bool operator!=(const eastl::allocator) { return true; }
	inline bool operator!=(const heap_allocator) { return false; }
	inline bool operator==(const eastl::allocator) { return false; }
	inline bool operator==(const heap_allocator) { return true; }

	inline void* allocate(size_t n) const
    {
    #ifdef USE_ZONE
        return Z_Malloc(n, TAG_STATIC, NULL, "allocator");
    #else
        return Mem_Alloc(n);
    #endif
    }
	inline void* allocate(size_t& n, size_t& alignment, size_t& offset) const
    {
    #ifdef USE_ZONE
        return Z_Malloc(n, TAG_STATIC, NULL, "allocator");
    #else
        return Mem_Alloc(n);
    #endif
    }
	inline void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags) const
    {
    #ifdef USE_ZONE
        return Z_Malloc(n, TAG_STATIC, NULL, "allocator");
    #else
        return Mem_Alloc(n);
    #endif
    }
	inline void deallocate(void *p, size_t) const noexcept
    {
    #ifdef USE_ZONE
        Z_Free(p);
    #else
        Mem_Free(p);
    #endif
    }
};

template <typename T>
struct heap_delete
{
	#if defined(EA_COMPILER_GNUC) && (EA_COMPILER_VERSION <= 4006) // GCC prior to 4.7 has a bug with noexcept here.
		EA_CONSTEXPR heap_delete() = default;
	#else
		EA_CONSTEXPR heap_delete() EA_NOEXCEPT = default;
	#endif

	template <typename U>  // Enable if T* can be constructed with U* (i.e. U* is convertible to T*).
	heap_delete(const heap_delete<U>&, typename eastl::enable_if<eastl::is_convertible<U*, T*>::value>::type* = 0) EA_NOEXCEPT {}

	void operator()(T* p) const EA_NOEXCEPT
	{
		static_assert(eastl::internal::is_complete_type_v<T>, "Attempting to call the destructor of an incomplete type");
    #ifdef USE_ZONE
        Z_Free(p);
    #else
        Mem_Free(p);
    #endif
	}
};

template<typename T>
inline void Deallocate(object_ptr_t<T> ptr)
{
    ptr->~T();
#ifdef USE_ZONE
    Z_Free(ptr);
#else
    Mem_Free(ptr);
#endif
}

template<typename T>
inline object_ptr_t<T> Allocate(void)
{
#ifdef USE_ZONE
    T *mem = (T *)Z_Malloc(sizeof(T), TAG_STATIC, &mem, "zalloc");
#else
    T *mem = (T *)Mem_Alloc(sizeof(T));
#endif
    ::new (mem) T();
    return mem;
}
template<typename T, typename... Args>
inline object_ptr_t<T> Allocate(Args&&... args)
{
#ifdef USE_ZONE
    T *mem = (T *)Z_Malloc(sizeof(T), TAG_STATIC, &mem, "zalloc");
#else
    T *mem = (T *)Mem_Alloc(sizeof(T));
#endif
    ::new (mem) T(eastl::forward<Args>(args)...);
    return mem;
}

inline void *Malloc(uint32_t size)
{
#ifdef USE_ZONE
    return Z_Malloc(size, TAG_STATIC, NULL, "zalloc");
#else
    return Mem_Alloc(size);
#endif
}

inline void Free(void *ptr)
{
#ifdef USE_ZONE
    Z_Free(ptr);
#else
    Mem_Free(ptr);
#endif
}

#endif