#ifndef __DEFS__
#define __DEFS__

#pragma once

using json = nlohmann::json;
using string_t = eastl::basic_string<char, heap_allocator>;
template<typename T>
using vector_t = eastl::vector<T, heap_allocator>;
template<typename T>
using list_t = eastl::list<T, heap_allocator>;

#ifndef MAX_GDR_PATH
#define MAX_GDR_PATH 64
#endif
#ifdef PATH_MAX
#define MAX_OSPATH PATH_MAX
#else
#define MAX_OSPATH 256
#endif
#define MAX_VA_BUFFER 8192

#ifndef arraylen
#define arraylen(x) (sizeof((x))/sizeof((*x)))
#endif

#ifdef _WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef enum {qfalse = 0, qtrue = 1} qboolean;
typedef unsigned char byte;
#endif

#endif