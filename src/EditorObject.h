#ifndef __EDITOR_OBJECT__
#define __EDITOR_OBJECT__

#pragma once

#include <foonathan_memory/foonathan/memory/default_allocator.hpp>
#include <foonathan_memory/foonathan/memory/deleter.hpp>
#include <foonathan_memory/foonathan/memory/debugging.hpp>
#include <foonathan_memory/foonathan/memory/aligned_allocator.hpp>
#include <foonathan_memory/foonathan/memory/heap_allocator.hpp>
#include <foonathan_memory/foonathan/memory/memory_pool.hpp>
#include <foonathan_memory/foonathan/memory/new_allocator.hpp>
#include <foonathan_memory/foonathan/memory/smart_ptr.hpp>
#include <foonathan_memory/foonathan/memory/namespace_alias.hpp>
#include <foonathan_memory/foonathan/memory/container.hpp>
#include <foonathan_memory/foonathan/memory/std_allocator.hpp>

#define IMPL_TEMPLATE_TYPE(alias,...) \
    using alias = __VA_ARGS__;

template<typename T>
using pool_allocator = memory::std_allocator<T, memory::memory_pool<>>;

IMPL_TEMPLATE_TYPE(string_t, eastl::basic_string<char, pool_allocator<char>>)
template<typename T>
IMPL_TEMPLATE_TYPE(unique_ptr_t, eastl::unique_ptr<T, memory::allocator_deallocator<T, memory::memory_pool<>>>)
template<typename T>
IMPL_TEMPLATE_TYPE(vector_t, eastl::vector<T, pool_allocator<T>>)
template<typename Key, typename T>
IMPL_TEMPLATE_TYPE(hash_table_t, eastl::hash_map<Key, T, eastl::hash<Key>, eastl::equal_to<Key>, pool_allocator<T>, true>)

class CEditorObject
{
public:
    CEditorObject(void);
    ~CEditorObject();

    template<typename T>
    unique_ptr_t<T> make_unique(void) const
    { return memory::allocate_unique(allocPool); }
    template<typename T, typename... Args>
    unique_ptr_t<T> make_unique(Args&&... args) const
    { return memory::allocate_unique(allocPool, eastl::forward<Args>(args)...); }

    /*
    NOTE: this class uses new and delete overrides that always allocates on a 16-byte alignment
    */

    inline void *operator new(size_t n)
    { return allocPool.allocate_node(n, 16); }
    inline void *operator new[](size_t n)
    { return allocPool.allocate_node(n, 16); }
    inline void operator delete(void *ptr)
    { allocPool.deallocate_node(ptr); }
    inline void operator delete[](void *ptr)
    { allocPool.deallocate_node(ptr); }
private:
    static CEditorObject bigboi;
    memory::memory_pool<> allocPool;
};

#endif