#ifndef _COMMON_HPP_
#define _COMMON_HPP_

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "gl.h"
#include <SDL2/SDL.h>
#include <memory>
#include <string>
#include <fstream>
#include <unordered_map>
#include <glm/glm.hpp>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include <boost/thread.hpp>

#include <EASTL/string.h>
#include <EASTL/map.h>
#include <EASTL/unordered_map.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/array.h>
#include <EASTL/list.h>
#include <EASTL/vector.h>
#include <EASTL/stack.h>

void Mem_Init(void);
void Mem_Shutdown(void);
void *Mem_Alloc(uint32_t size);
void *Mem_Realloc(void *ptr, uint64_t nsize);

void *GetResizedMemory(void *ptr, uint64_t nsize);
void *GetClearedMemory(uint64_t size);
void *GetMemory(uint64_t size);
void FreeMemory(void *ptr);
char *CopyString(const char *s);

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
    { return GetMemory(n); }
	inline void* allocate(size_t& n, size_t& alignment, size_t& offset) const
    { return GetMemory(n); }
	inline void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags) const
    { return GetMemory(n); }
	inline void deallocate(void *p, size_t) const noexcept
    { FreeMemory(p); }
};

template<typename T>
class heap_allocator_template
{
public:
    heap_allocator_template(const char* _name = "allocator") noexcept { }
    template<typename U>
	heap_allocator_template(const heap_allocator_template<U> &) noexcept { }
    ~heap_allocator_template() { }

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

#include <nlohmann/json.hpp>
#include "defs.h"

#define PAD(base, alignment) (((base)+(alignment)-1) & ~((alignment)-1))
template<typename type, typename alignment>
INLINE type *PADP(type *base, alignment align)
{ return (type *)((void *)PAD((intptr_t)base, align)); }

void Error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void Printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
const char *CurrentDirName(void);
const char *va(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
uint64_t LoadFile(const char *filename, void **buffer);
bool LoadJSON(json& data, const std::string& path);
void Exit(void);
int GetParm(const char *parm);
bool N_strcat(char *dest, size_t size, const char *src);
int N_isprint(int c);
int N_isalpha(int c);
int N_isupper(int c);
int N_islower(int c);
bool N_isintegral(float f);
void N_strncpy (char *dest, const char *src, size_t count);
void N_strncpyz (char *dest, const char *src, size_t count);
bool N_isanumber(const char *s);
int N_stricmp( const char *s1, const char *s2 );
uint64_t LittleLong(uint64_t l);
uint32_t LittleInt(uint32_t l);
float LittleFloat(float f);
int N_stricmpn (const char *str1, const char *str2, size_t n);
const char *N_stristr(const char *s, const char *find);
#ifdef _WIN32
int N_vsnprintf( char *str, size_t size, const char *format, va_list ap );
#else
#define N_vsnprintf vsnprintf
#endif
uint32_t StrToFilter(const char *str);
uint32_t StrToWrap(const char *str);
uint32_t StrToFormat(const char *str);
const char *FilterToString(uint32_t filter);
const char *FormatToString(uint32_t filter);
const char *WrapToString(uint32_t filter);
const char *GetExtension( const char *name );
void StripExtension(const char *in, char *out, uint64_t destsize);
void DefaultExtension( char *path, uint64_t maxSize, const char *extension );
void SafeWrite(const void *buffer, size_t size, FILE *fp);
void SafeRead(void *buffer, size_t size, FILE *fp);
FILE *SafeOpenRead(const char *path);
FILE *SafeOpenWrite(const char *path);
bool FileExists(const char *path);
uint64_t FileLength(FILE *fp);
const char* GetFilename(const char *path);
bool IsAbsolutePath(const char *path);
INLINE const char *GetAbsolutePath(const char *path)
{ return IsAbsolutePath(path) ? path : GetFilename(path); }
char* BuildOSPath(const char *curPath, const char *gamepath, const char *npath);

typedef struct {
    int i;
    const char *s;
} StringToInt_t;

INLINE int StringToInt(const std::string& from, const StringToInt_t *to, int nElems)
{
    for (int i = 0; i < nElems; i++) {
        if (!N_stricmp(to[i].s, from.c_str())) {
            return to[i].i;
        }
    }
    return -1;
}

inline constexpr const StringToInt_t texture_details[] = {
    {0, "GPU vs God"},
    {1, "Expensive Shit We've Got Here"},
    {2, "Normie"},
    {3, "Integrated GPU"}
};

inline constexpr const StringToInt_t texture_filters[] = {
    {0, "Nearest"},
    {1, "Linear"},
    {2, "Bilinear"},
    {3, "Trilinear"}
};

inline constexpr const StringToInt_t texture_filters_alt[] = {
    {0, "GL_NEAREST | GL_NEAREST | (Nearest)"},
    {1, "GL_LINEAR  | GL_LINEAR  | (Linear)"},
    {2, "GL_NEAREST | GL_LINEAR  | (Bilinear)"},
    {3, "GL_LINEAR  | GL_NEAREST | (Trilinear)"}
};


/*
All the random shit I pulled from GtkRadiant into this project
*/
#include "idatastream.h"
#include "stream.h"
#include "list.h"
#include "idll.h"

/*
Editor-specific stuff
*/
#include "widget.h"
#include "gln_files.h"
#include "command.h"
#include "events.h"
#include "gui.h"
#include "preferences.h"
#include "editor.h"
#include "ImGuiFileDialog.h"
#include "entity.h"
#include "map.h"
#include "parse.h"

#if 0
#undef new
#undef delete

INLINE void *operator new(size_t size) noexcept
{ return GetClearedMemory(size); }
INLINE void *operator new(size_t size, const std::nothrow_t&) noexcept
{ return ::operator new(size); }
INLINE void *operator new[](size_t size) noexcept
{ return ::operator new(size); }
INLINE void *operator new[](size_t size, const std::nothrow_t&) noexcept
{ return ::operator new[](size); }
INLINE void *operator new(size_t size, std::align_val_t alignment) noexcept
{ (void)alignment; return ::operator new(size); }
INLINE void *operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{ return ::operator new(size, alignment); }
INLINE void *operator new[](size_t size, std::align_val_t alignment) noexcept
{ return ::operator new(size, alignment); }
INLINE void *operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{ return ::operator new[](size, alignment); }

INLINE void operator delete(void* ptr) noexcept
{
    if (ptr)
        FreeMemory(ptr);
}
INLINE void operator delete(void* ptr, const std::nothrow_t&) noexcept
{ ::operator delete(ptr); }
INLINE void operator delete(void* ptr, size_t) noexcept
{ ::operator delete(ptr); }
INLINE void operator delete[] (void* ptr) noexcept
{ ::operator delete(ptr); }
INLINE void operator delete[] (void* ptr, const std::nothrow_t&) noexcept
{ ::operator delete[](ptr); }
INLINE void operator delete[] (void* ptr, size_t) noexcept
{ ::operator delete[](ptr); }
INLINE void operator delete(void* ptr, std::align_val_t) noexcept
{ ::operator delete(ptr); }
INLINE void operator delete(void* ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept
{ ::operator delete(ptr, alignment); }
INLINE void operator delete(void* ptr, size_t, std::align_val_t alignment) noexcept
{ ::operator delete(ptr, alignment); }
INLINE void operator delete[] (void* ptr, std::align_val_t alignment) noexcept
{ ::operator delete(ptr, alignment); }
INLINE void operator delete[] (void* ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept
{ ::operator delete[](ptr, alignment); }
INLINE void operator delete[] (void* ptr, size_t, std::align_val_t alignment) noexcept
{ ::operator delete[](ptr, alignment); }
#endif

extern int parm_compression;
extern int myargc;
extern char **myargv;

typedef enum {
    // image data
    BUFFER_JPEG = 0,
    BUFFER_PNG,
    BUFFER_BMP,
    BUFFER_TGA,

    // audio data
    BUFFER_OGG,
    BUFFER_WAV,
    
    // simple data
    BUFFER_DATA,
    
    // text
    BUFFER_TEXT,
} bufferType_t;

char *Compress(void *buf, uint64_t buflen, uint64_t *outlen, int compression = parm_compression);
char *Decompress(void *buf, uint64_t buflen, uint64_t *outlen, int compression = parm_compression);

#endif