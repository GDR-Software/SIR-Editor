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
#include <spdlog.h> // spdlog/
#include <async_logger.h> // spdlog/
#include <logger.h> // spdlog/
#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/map.h>
#include <filesystem>
#include <set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern int myargc;
extern char **myargv;

void *SafeMalloc(size_t size);
char* BuildOSPath(const std::filesystem::path& curPath, const eastl::string& gamepath, const char *npath);
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
bool N_isanumber(const char *s);
int N_stricmp( const char *s1, const char *s2 );
int N_stricmpn (const char *str1, const char *str2, size_t n);
#ifdef _WIN32
int N_vsnprintf( char *str, size_t size, const char *format, va_list ap );
#else
#define N_vsnprintf vsnprintf
#endif

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