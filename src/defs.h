#ifndef __DEFS__
#define __DEFS__

#pragma once

#ifdef _WIN32
#define T TEXT
#ifdef UNICODE
inline LPWSTR AtoW( const char *s ) 
{
	static WCHAR buffer[MAXPRINTMSG*2];
	MultiByteToWideChar( CP_ACP, 0, s, strlen( s ) + 1, (LPWSTR) buffer, arraylen( buffer ) );
	return buffer;
}

inline const char *WtoA( const LPWSTR s ) 
{
	static char buffer[MAXPRINTMSG*2];
	WideCharToMultiByte( CP_ACP, 0, s, -1, buffer, arraylen( buffer ), NULL, NULL );
	return buffer;
}
#else
#define AtoW(S) (S)
#define WtoA(S) (S)
#endif
#endif

#ifndef INLINE
    #ifdef _MSC_VER
        #define INLINE __forceinline
    #elif defined(__GNUC__) || defined(__MINGW32__) || defined(__MINGW64__)
        #define INLINE __attribute__((always_inline)) inline
    #else
        #ifdef __cplusplus
            #if __cplusplus >= 201703L
                #define INLINE [[gnu::always_inline]] inline
            #else
                #define INLINE __inline
            #endif
        #else
            #define INLINE __inline
        #endif
    #endif
#endif

using json = nlohmann::json;
using string_t = eastl::basic_string<char, heap_allocator>;
template<typename T>
using vector_t = eastl::vector<T, heap_allocator>;
template<typename T>
using list_t = eastl::list<T, heap_allocator>;
template<typename T>
using stack_t = eastl::stack<T, vector_t<T>>;

#ifndef MAX_GDR_PATH
#define MAX_GDR_PATH 64
#endif
#ifdef PATH_MAX
#define MAX_OSPATH PATH_MAX
#else
#define MAX_OSPATH 256
#endif
#define MAX_VA_BUFFER 8192

#ifdef _WIN32
#define EXE_EXT ".exe"
#else
// with unix apps, there isn't really a specific or single app exe, so we'll just go with an empty string
#define EXE_EXT ""
#endif

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

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