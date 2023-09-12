#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "bff.h"

#define BFF_HEADER_MAGIC 0x5f3759df

#define PAD(base,alignment) (((base)+(alignment)-1)&~((alignment)-1))

#define BFF_VERSION_MAJOR 0
#define BFF_VERSION_MINOR 1
#define BFF_VERSION ((BFF_VERSION_MAJOR<<8)+BFF_VERSION_MINOR)

typedef struct
{
    uint64_t magic;
    uint64_t version;
    uint64_t numChunks;
    uint64_t namelen;
} bffHeader_t;

typedef enum
{
    BFF_FILE_READ,
    BFF_FILE_WRITE
} bffFileMode_t;

struct bffChunk_s
{
    char *name;
    void *buf;
    uint64_t namelen;
    uint64_t size;
    uint64_t fileofs;
    bffChunkType_t type;
};

struct bffFile_s
{
    char *name;
    bffHeader_t header;
    bffChunk_t *chunkList;
    bffFileMode_t mode;
};

static void *Default_Alloc(uint64_t size, void *user_data)
{
    return malloc(PAD(size, 64));
}

static void Default_Free(void *ptr, void *user_data)
{
    free(ptr);
}

typedef struct {
    bffAlloc BFF_Alloc = Default_Alloc;
    bffFree BFF_Free = Default_Free;
    void *userData = NULL;
    uint64_t allocAlignment = 64;
} bffConfig_t;

static bffError lastError = BFF_NOERROR;
static bffConfig_t conf;

typedef unsigned char byte;
typedef struct {
    byte *buf;
    uint64_t usedBytes;
    uint64_t length;
} bffBuffer_t;

static inline void ReadBytes(void *data, uint64_t size, bffBuffer_t *buf)
{
    if (buf->usedBytes + size > buf->length) {
        lastError = BFF_BUFFER_OVERREAD;
        return;
    }
    memcpy(data, &buf->buf[buf->usedBytes], size);
    buf->usedBytes += size;
}

static inline void SafeRead(void *buffer, uint64_t size, FILE *fp)
{
    if (lastError != BFF_NOERROR)
        return;
    if (!fread(buffer, size, 1, fp))
        lastError = BFF_BAD_READ;
}

static inline bffError ValidateHeader(const bffHeader_t *header)
{
    if (header->magic != BFF_HEADER_MAGIC) 
        return lastError = BFF_BAD_HEADER;
    if (header->version != BFF_VERSION)
        return lastError = BFF_BAD_HEADER;
    if (!header->numChunks)
        return lastError = BFF_BAD_HEADER;
    if (!header->namelen)
        return lastError = BFF_BAD_HEADER;
    
    return lastError = BFF_NOERROR;
}

void BFF_Conf_SetAllocAlignment(uint64_t alignment)
{
    switch (alignment) {
    case 8:
    case 16:
    case 24:
    case 32:
    case 64:
    case 72:
    case 128:
        conf.allocAlignment = alignment;
        break;
    default:
        lastError = BFF_INVALID_PARAM;
        return;
    };
}

void BFF_SetMemoryFuncs(bffAlloc allocFunc, bffFree freeFunc, void *user_data = NULL)
{
    conf.BFF_Alloc = allocFunc;
    conf.BFF_Free = freeFunc;
    conf.userData = user_data;
}

bffError BFF_GetLastError(void)
{
    return lastError;
}

const char *BFF_GetErrorString(bffError err)
{
    switch (err) {
    case BFF_BAD_HEADER: return "bad bff header magic or version";
    case BFF_BAD_READ: return "fread() failed";
    case BFF_FAILED_FILE_OPEN: return "fopen() failed on given path";
    case BFF_INVALID_PARAM: return "invalid parameter given to api function";
    case BFF_OUT_OF_MEM: return "allocation failed (returned NULL)";
    case BFF_BUFFER_OVERREAD: return "read more bytes than available in buffer";
    case BFF_NOERROR:
        break;
    };
    return "No Error";
}

bffFile_t *BFF_OpenFileRead(const char *path)
{
    FILE *fp;
    bffHeader_t header;
    bffChunk_t *chunk, readChunk;
    bffFile_t *bff;
    bffBuffer_t buf;
    char *nameptr;
    uint64_t size, i;
    uint64_t pos;
    uint64_t filelength;

    if (lastError != BFF_NOERROR)
        return NULL;
    
    if (!path || !path[0]) {
        lastError = BFF_INVALID_PARAM;
        return NULL;
    }

    fp = fopen(path, "rb");
    if (!fp) {
        lastError = BFF_FAILED_FILE_OPEN;
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    filelength = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    memset(&buf, 0, sizeof(buf));
    buf.buf = (byte *)conf.BFF_Alloc(filelength, conf.userData);
    if (!buf.buf) {
        lastError = BFF_OUT_OF_MEM;
        fclose(fp);
        return NULL;
    }
    SafeRead(buf.buf, filelength, fp);
    fclose(fp);

    ReadBytes(&header, sizeof(header), &buf);
    
    if (ValidateHeader(&header) != BFF_NOERROR)
        return NULL;
    
    buf.usedBytes += header.namelen;
    size = 0;
    size += PAD(sizeof(*bff), conf.allocAlignment);
    size += PAD(header.namelen, conf.allocAlignment);
    size += PAD(sizeof(*bff->chunkList) * header.numChunks, conf.allocAlignment);

    for (i = 0; i < header.numChunks; i++) {
        ReadBytes(&readChunk.size, sizeof(uint64_t), &buf);
        ReadBytes(&readChunk.type, sizeof(bffChunkType_t), &buf);
        ReadBytes(&readChunk.fileofs, sizeof(uint64_t), &buf);
        ReadBytes(&readChunk.namelen, sizeof(uint64_t), &buf);

        size += PAD(readChunk.namelen, conf.allocAlignment);
        size += PAD(readChunk.size, conf.allocAlignment);

        // skip the name, we'll read it later
        pos = buf.usedBytes + readChunk.namelen;
        buf.usedBytes += readChunk.namelen;
    }

    size = PAD(size, conf.allocAlignment);

    bff = (bffFile_t *)conf.BFF_Alloc(size, conf.userData);
    memset(bff, 0, size);
    bff->name = (char *)(bff + 1);
    bff->chunkList = (bffChunk_t *)(bff->name + header.namelen);

    memcpy(&bff->header, &header, sizeof(header));
    bff->mode = BFF_FILE_READ;

    if (!bff->chunkList) {
        lastError = BFF_OUT_OF_MEM;
        conf.BFF_Free(bff, conf.userData);
        return NULL;
    }

    chunk = bff->chunkList;
    nameptr = (char *)(bff->chunkList + header.numChunks);

    buf.usedBytes = sizeof(header);
    ReadBytes(bff->name, header.namelen, &buf);
    
    for (uint64_t i = 0; i < bff->header.numChunks; i++) {
        ReadBytes(&chunk->size, sizeof(uint64_t), &buf);
        ReadBytes(&chunk->type, sizeof(bffChunkType_t), &buf);
        ReadBytes(&chunk->fileofs, sizeof(uint64_t), &buf);
        ReadBytes(&chunk->namelen, sizeof(uint64_t), &buf);

        chunk->name = nameptr;
        ReadBytes(chunk->name, chunk->namelen, &buf);
        ReadBytes(chunk->buf, chunk->size, &buf);

        nameptr = chunk->name + chunk->namelen;
    }

    conf.BFF_Free(buf.buf, conf.userData);

    return bff;
}

bffError BFF_GetChunkList(char **list, uint64_t *numChunks)
{

}

bffError BFF_ReadChunk(bffChunk_t *chunk, const char *name);
bffError BFF_GetChunkInfo(bffChunk_t *chunk, bffChunkInfo_t *info);

bffFile_t *BFF_OpenFileWrite(const char *path)
{

}

void BFF_CloseFile(bffFile_t *file)
{
    conf.BFF_Free(file, conf.userData);
}
