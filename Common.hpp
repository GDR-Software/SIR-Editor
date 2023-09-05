#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/hash_map.h>
#include <EASTL/unordered_map.h>
#include <filesystem>
#include <set>
#include "Zone.h"
#include <foonathan_memory/foonathan/memory/namespace_alias.hpp>
#include <foonathan_memory/foonathan/memory/static_allocator.hpp>
#include <foonathan_memory/foonathan/memory/container.hpp>
#include <foonathan_memory/foonathan/memory/memory_pool.hpp>

/*
spdlog and libfmt don't get along well if the path isn't set to /usr/local/include
*/
#include <spdlog.h> // spdlog/
#include <async_logger.h> // spdlog/
#include <logger.h> // spdlog/

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


using path_t = std::filesystem::path;
using directory_entry_t = std::filesystem::directory_entry;
using directory_iterator_t = std::filesystem::directory_iterator;

template<typename T>
using object_ptr_t = T*;
template<typename T>
using unique_ptr_t = eastl::unique_ptr<T, heap_delete<T>>;
using string_t = eastl::basic_string<char, heap_allocator>;
template<typename T>
using vector_t = eastl::vector<T, heap_allocator>;

//template <typename Key, typename T, typename Hash = eastl::hash<Key>, typename Predicate = eastl::equal_to<Key>, 
//			  typename Allocator = EASTLAllocatorType, bool bCacheHashCode = false>
template<typename Key, typename T>
using hash_table_t = eastl::hash_map<Key, T, eastl::hash<Key>, eastl::equal_to<Key>, heap_allocator, true>;
template<typename T>
using string_hash_t = hash_table_t<eastl::string, T>;

template<typename T>
struct heap_allocator_template;

#include <nlohmann/json.hpp>

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

#define PAD(base, alignment) (((base)+(alignment)-1) & ~((alignment)-1))
template<typename type, typename alignment>
inline type *PADP(type *base, alignment align)
{
    return (type *)((void *)PAD((intptr_t)base, align));
}

typedef unsigned char byte;

using json = nlohmann::json;

extern int myargc;
extern char **myargv;

const char* GetFilename(const char *path);
bool IsAbsolutePath(const string_t& path);

inline const char *GetAbsolutePath(const path_t& path)
{ return IsAbsolutePath(path.c_str()) ? path.c_str() : GetFilename(path.c_str()); }

void *SafeMalloc(size_t size);
char* BuildOSPath(const path_t& curPath, const string_t& gamepath, const char *npath);
void Exit(void);
void Error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void Printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int GetParm(const char *parm);
void N_strcat(char *dest, size_t size, const char *src);
int N_isprint(int c);
int N_isalpha(int c);
int N_isupper(int c);
int N_islower(int c);
bool N_isintegral(float f);
void N_strncpy (char *dest, const char *src, size_t count);
void N_strncpyz (char *dest, const char *src, size_t count);
bool N_isanumber(const char *s);
int N_stricmp( const char *s1, const char *s2 );
int N_stricmpn (const char *str1, const char *str2, size_t n);
const char *N_stristr(const char *s, const char *find);
#ifdef _WIN32
int N_vsnprintf( char *str, size_t size, const char *format, va_list ap );
#else
#define N_vsnprintf vsnprintf
#endif
const char *COM_GetExtension( const char *name );
void COM_StripExtension(const char *in, char *out, uint64_t destsize);

void SafeWrite(const void *buffer, size_t size, FILE *fp);
void SafeRead(void *buffer, size_t size, FILE *fp);
FILE *SafeOpenRead(const char *path);
FILE *SafeOpenWrite(const char *path);
bool FileExists(const char *path);
uint64_t FileLength(FILE *fp);

void TokenizeString(const char *str, bool ignoreQuotes);
uint32_t Argc(void);
const char *Argv(uint32_t index);

#include "keycodes.h"

typedef struct
{
    int x;
    int y;
    bool moving;
    bool wheelUp;
    bool wheelDown;
    float yaw, pitch;
    float angle;
    float deltaX, deltaY;
} mouse_t;
extern mouse_t mouse;

void InitEvents(void);
uint64_t EventLoop(void);
bool Key_IsDown(int keynum);
void Mouse_GetPos(int *x, int *y);
bool Mouse_WheelUp(void);
bool Mouse_WheelDown(void);

typedef struct
{
	bool down;
	bool bound;
	uint32_t repeats;
	char *binding;
} nkey_t;

extern nkey_t keys[NUMKEYS];

#define arraylen(x) (sizeof((x))/sizeof((*x)))

#endif