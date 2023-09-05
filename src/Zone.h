#ifndef __ZONE__
#define __ZONE__

#pragma once

enum {
	TAG_FREE		= 0, // a free block
	TAG_STATIC		= 1, // stays allocated for entirety (or until explicitly free) of execution time
	TAG_LEVEL		= 2, // level-scoped allocations
	TAG_RENDERER	= 3, // allocations made from any of the rendering libraries
	TAG_SFX			= 4, // general sound allocations
	TAG_MUSIC		= 5, // music allocations
	TAG_PROJECT		= 6,
	TAG_SEARCH_PATH	= 7, // a filesystem searchpath
	TAG_BFF			= 8, // a bff archive file
	TAG_HUNK		= 9, // allocated with temp hunk
	TAG_PURGELEVEL	= 100, // purgeable block
	TAG_CACHE		= 101, // cached block, migh be used in the future, but can also be purged
};

#define NUMTAGS 11

#define TAG_SCOPE TAG_CACHE
#define TAG_LOAD TAG_CACHE

void* Z_SMalloc(uint32_t size, int tag, void *user, const char *name);
void* Z_Malloc(uint32_t size, int tag, void *user, const char *name);
void* Z_Calloc(uint32_t size, int tag, void *user, const char *name);
void* Z_Realloc(void *ptr, uint32_t nsize, int tag, void *user, const char *name);
char* Z_Strdup(const char *str);
void Z_Free(void *ptr);
char* Z_StrdupTag(const char *str, int tag);

void Z_Shutdown(void);
void Z_FreeTags(int lowtag, int hightag);
void Z_ChangeTag(void* user, int tag);
void Z_ChangeUser(void* newuser, void* olduser);
void Z_ChangeName(void* user, const char* name);
void Z_CleanCache(void);
void Z_CheckHeap(void);
void Z_ClearZone(void);
void Z_Print(bool all);
void Z_Init(void);
uint64_t Z_FreeMemory(void);
void* Z_ZoneBegin(void);
void* Z_ZoneEnd(void);
uint64_t Z_BlockSize(void *p);
uint32_t Z_NumBlocks(int tag);
void Z_TouchMemory(uint64_t *sum);

template<class T>
struct zone_allocator
{
	constexpr zone_allocator(void) noexcept { }
	constexpr zone_allocator(const char* name = "zallocator") noexcept { }

	typedef T value_type;
	template<class U>
	constexpr zone_allocator(const zone_allocator<U> &) noexcept { }

	constexpr inline bool operator!=(const eastl::allocator) { return true; }
	constexpr inline bool operator!=(const zone_allocator) { return false; }
	constexpr inline bool operator==(const eastl::allocator) { return false; }
	constexpr inline bool operator==(const zone_allocator) { return true; }

	inline void* allocate(size_t n) const
	{ return n <= 64 ? Z_SMalloc(n, TAG_STATIC, NULL, "zalloc") : Z_Malloc(n, TAG_STATIC, NULL, "zalloc"); }
	inline void* allocate(size_t& n, size_t& alignment, size_t& offset) const
	{ return n <= 64 ? Z_SMalloc(n, TAG_STATIC, NULL, "zalloc") : Z_Malloc(n, TAG_STATIC, NULL, "zalloc"); }
	inline void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags) const
	{ return n <= 64 ? Z_SMalloc(n, TAG_STATIC, NULL, "zalloc") : Z_Malloc(n, TAG_STATIC, NULL, "zalloc"); }
	inline void deallocate(void *p, size_t) const noexcept
	{ Z_Free(p); }
};

template<int tag>
class zone_allocator_tag
{
public:
	zone_allocator_tag(const char* name = "zallocator") noexcept { }

	constexpr zone_allocator_tag(const zone_allocator_tag &) noexcept { }

	inline bool operator!=(const eastl::allocator) { return true; }
	inline bool operator!=(const zone_allocator_tag) { return false; }
	inline bool operator==(const eastl::allocator) { return false; }
	inline bool operator==(const zone_allocator_tag) { return true; }

	inline void* allocate(size_t n) const
	{ return n <= 64 ? Z_SMalloc(n, tag, NULL, "zalloc") : Z_Malloc(n, tag, NULL, "zalloc"); }
	inline void* allocate(size_t& n, size_t& alignment, size_t& offset) const
	{ return n <= 64 ? Z_SMalloc(n, tag, NULL, "zalloc") : Z_Malloc(n, tag, NULL, "zalloc"); }
	inline void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags) const
	{ return n <= 64 ? Z_SMalloc(n, tag, NULL, "zalloc") : Z_Malloc(n, tag, NULL, "zalloc"); }
	inline void deallocate(void *p, size_t) const noexcept
	{ Z_Free(p); }
};

class zone_allocator_notemplate
{
public:
	zone_allocator_notemplate(const char* name = "zallocator") noexcept { }

	constexpr zone_allocator_notemplate(const zone_allocator_notemplate &) noexcept { }

	inline bool operator!=(const eastl::allocator) { return true; }
	inline bool operator!=(const zone_allocator_notemplate) { return false; }
	inline bool operator==(const eastl::allocator) { return false; }
	inline bool operator==(const zone_allocator_notemplate) { return true; }

	inline void* allocate(size_t n) const
	{ return n <= 64 ? Z_SMalloc(n, TAG_STATIC, NULL, "zalloc") : Z_Malloc(n, TAG_STATIC, NULL, "zalloc"); }
	inline void* allocate(size_t& n, size_t& alignment, size_t& offset) const
	{ return n <= 64 ? Z_SMalloc(n, TAG_STATIC, NULL, "zalloc") : Z_Malloc(n, TAG_STATIC, NULL, "zalloc"); }
	inline void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags) const
	{ return n <= 64 ? Z_SMalloc(n, TAG_STATIC, NULL, "zalloc") : Z_Malloc(n, TAG_STATIC, NULL, "zalloc"); }
	inline void deallocate(void *p, size_t) const noexcept
	{ Z_Free(p); }
};

#endif
