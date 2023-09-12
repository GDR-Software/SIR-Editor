#ifndef __BFF_H__
#define __BFF_H__

#pragma once

typedef enum : uint64_t
{
    CHUNK_TYPE_TEXTURE,
    CHUNK_TYPE_AUDIO,
    CHUNK_TYPE_SOUND,
} bffChunkType_t;

typedef struct
{
    const char *name;
    uint64_t namelen;
    uint64_t size;
    bffChunkType_t type;
} bffChunkInfo_t;

typedef struct bffFile_s bffFile_t;
typedef struct bffChunk_s bffChunk_t;

#define BFF_NOERROR (1)
#define BFF_OUT_OF_MEM (-1)
#define BFF_BAD_HEADER (-2)
#define BFF_BAD_READ (-3)
#define BFF_FAILED_FILE_OPEN (-4)
#define BFF_INVALID_PARAM (-5)
#define BFF_BUFFER_OVERREAD (-6)

typedef int bffError;
typedef void *(*bffAlloc)(uint64_t size, void *user_data);
typedef void (*bffFree)(void *ptr, void *user_data);

void BFF_Conf_SetAllocAlignment(uint64_t alignment);
void BFF_SetMemoryFuncs(bffAlloc allocFunc, bffFree freeFunc, void *user_data = NULL);
bffError BFF_GetLastError(void);
const char *BFF_GetErrorString(bffError err);
void BFF_CloseFile(bffFile_t *file);
bffError BFF_GetChunkList(char **list, uint64_t *numChunks);
bffError BFF_GetChunkInfo(bffChunk_t *chunk, bffChunkInfo_t *info);

bffFile_t *BFF_OpenFileRead(const char *path);
bffError BFF_ReadChunk(bffChunk_t *chunk, void *buffer, uint64_t size);

bffFile_t *BFF_OpenFileWrite(const char *path);
bffError BFF_WriteChunk(bffChunk_t *chunk, const void *buffer, uint64_t size);

#endif