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
#include <EASTL/queue.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/hash_map.h>
#include <EASTL/unordered_map.h>
#include <filesystem>
#include <set>
#include <SDL2/SDL.h>
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



template<typename T>
using object_ptr_t = T*;

#include "Alloc.h"

using path_t = std::filesystem::path;
using directory_entry_t = std::filesystem::directory_entry;
using directory_iterator_t = std::filesystem::directory_iterator;

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

#include "gln_files.h"

#define PAD(base, alignment) (((base)+(alignment)-1) & ~((alignment)-1))
template<typename type, typename alignment>
inline type *PADP(type *base, alignment align)
{
    return (type *)((void *)PAD((intptr_t)base, align));
}

using json = nlohmann::json;

extern int myargc;
extern char **myargv;

const char* GetFilename(const char *path);
bool IsAbsolutePath(const string_t& path);

inline const char *GetAbsolutePath(const path_t& path)
{ return IsAbsolutePath(path.c_str()) ? path.c_str() : GetFilename(path.c_str()); }

#ifdef PATH_MAX
#define MAX_OSPATH PATH_MAX
#else
#define MAX_OSPATH 256
#endif
#define VA_BUF_SIZE 8192
inline const char *va(const char *fmt, ...)
{
    va_list argptr;
    static char string[2][VA_BUF_SIZE];
    static int index = 0;
    char *buf;

    buf = string[index & 1];
    index++;

    va_start(argptr, fmt);
    vsprintf(buf, fmt, argptr);
    va_end(argptr);

    return buf;
}
uint64_t LoadFile(const char *filename, void **buffer);
void *SafeMalloc(size_t size);
char* BuildOSPath(const path_t& curPath, const string_t& gamepath, const char *npath);
void Exit(void);
void Error(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void Printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
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
const char *COM_GetExtension( const char *name );
void COM_StripExtension(const char *in, char *out, uint64_t destsize);
void COM_DefaultExtension( char *path, uint64_t maxSize, const char *extension );

void SafeWrite(const void *buffer, size_t size, FILE *fp);
void SafeRead(void *buffer, size_t size, FILE *fp);
FILE *SafeOpenRead(const char *path);
FILE *SafeOpenWrite(const char *path);
bool FileExists(const char *path);
uint64_t FileLength(FILE *fp);

void TokenizeString(const char *str, bool ignoreQuotes);
uint32_t Argc(void);
const char *Argv(uint32_t index);

#include "MapFile.h"
#include "EditorTool.h"
#include "Texture.h"
#include "Tileset.h"
#include "Map.h"
#include "Project.h"
#include "EditorManager.h"
#include "TilesetManager.h"
#include "ProjectManager.h"
#include "TextureManager.h"
#include "MapManager.h"

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

typedef void (*cmdfunc_t)(void);

void Cmd_Init(void);
void Cmd_Execute(const char *text);
void Cmd_AddCommand(const char *name, cmdfunc_t func);
void Cmd_RemoveCommand(const char *name);
void Cmd_Shutdown(void);

extern nkey_t keys[NUMKEYS];

#define arraylen(x) (sizeof((x))/sizeof((*x)))
#define COMPRESS_ZLIB 0
#define COMPRESS_BZIP2 1

extern int parm_saveJsonMaps;
extern int parm_saveJsonTilesets;
extern int parm_useInternalTilesets;
extern int parm_useInternalMaps;
extern int parm_compression;

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