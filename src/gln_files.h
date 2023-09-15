#ifndef __GLN_FILES__
#define __GLN_FILES__

#pragma once

/*
GLN_FILES: these definitions must not change in any glnomad extension or project
*/


#ifndef MAX_GDR_PATH
#define MAX_GDR_PATH 64
#endif

// the minimum size in bytes a lump should be before compressing it
#define COMPRESSED_LUMP_SIZE 2048

#define COMPRESS_NONE 0
#define COMPRESS_ZLIB 1
#define COMPRESS_BZIP2 2

#define TEXTURE_FILE_EXT ".tex2d"
#define TILESET_FILE_EXT ".tile2d"
#define MAP_FILE_EXT ".map"
#define ANIMATION_FILE_EXT ".anim2d"
#define TEXTURE_FILE_EXT_RAW "tex2d"
#define TILESET_FILE_EXT_RAW "tile2d"
#define MAP_FILE_EXT_RAW "map"
#define ANIMATION_FILE_EXT_RAW "anim2d"
#define LEVEL_FILE_EXT ".bmf"
#define LEVEL_FILE_EXT_RAW "bmf"

typedef struct {
    uint64_t fileofs;
    uint64_t length;
} lump_t;

#define TEX2D_IDENT (('D'<<16)+('2'<<8)+'T')
#define TEX2D_VERSION 1

typedef struct {
    uint64_t ident;
    uint64_t version;

    char name[MAX_GDR_PATH];
    uint32_t minfilter;
    uint32_t magfilter;
    uint32_t wrapS;
    uint32_t wrapT;
    uint32_t multisampling;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t format; // OpenGL internal format
    uint32_t compression; // zlib or bzip2
    uint64_t compressedSize;
    uint64_t fileSize; // og file size
} tex2d_t;

#define TILE2D_MAGIC 0xfda218891
#define TILE2D_VERSION 0

typedef struct {
    uint32_t texIndex;
    float texcoords[4][2];
} tile2d_sprite_t;

typedef struct {
    uint32_t numTiles;
    uint32_t tileWidth;
    uint32_t tileHeight;
    uint32_t hasTexture; // can only be false in the map editor
    uint32_t compression; // only for sprites
    uint64_t compressedSize; // only for sprites
    char texture[MAX_GDR_PATH]; // store a texture inside of a tileset
} tile2d_info_t;

typedef struct {
    uint64_t magic;
    uint64_t version;
    tile2d_info_t info;
    tile2d_sprite_t *sprites;
} tile2d_header_t;

#define ANIM2D_IDENT (('D'<<24)+('2'<<16)+('A'<<8)+'#')
#define ANIM2D_VERSION 1

typedef struct {
    uint32_t index;
    uint32_t numtics;
} anim2d_frame_t;

typedef struct {
    uint64_t ident;
    uint64_t version;
    uint64_t numFrames;
    anim2d_frame_t *frames;
} anim2d_header_t;

#define MAP_IDENT (('#'<<24)+('P'<<16)+('A'<<8)+'M')
#define MAP_VERSION 1

#define MAX_MAP_SPAWNS 0x8000
#define MAX_MAP_CHECKPOINTS 0x200
#define MAX_MAP_LIGHTS 0x800

#define MAX_MAP_TILES 0x800000
#define MAX_MAP_VERTICES (MAX_MAP_TILES*4)
#define MAX_MAP_INDICES (MAX_MAP_TILES*6)

#define LUMP_TILES 0
#define LUMP_CHECKPOINTS 1
#define LUMP_SPAWNS 2
#define LUMP_LIGHTS 3
#define LUMP_VERTICES 4
#define LUMP_INDICES 5
#define LUMP_TILESET 6

#define NUMLUMPS 7

typedef struct {
    float brightness;
    float radius;
    float origin[3];
    float color[4];
} maplight_t;

typedef struct {
    uint32_t index;
    uint32_t pos[2];
    float **texcoords;
} maptile_t;

typedef struct {
    float pos[2];
    float texcoords[2];
    float sprite;
    byte color[4];
} mapvert_t;

typedef struct {
    char name[MAX_GDR_PATH];
    uint32_t minfilter;
    uint32_t magfilter;
    uint32_t wrapS;
    uint32_t wrapT;
    uint32_t multisampling;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t format; // OpenGL internal format
    uint32_t compression; // zlib or bzip2
    uint64_t compressedSize;
} maptexture_t;

typedef struct {
    char name[MAX_GDR_PATH];
    uint32_t numTiles;
    uint32_t tileWidth;
    uint32_t tileHeight;
    uint32_t compression;
    uint64_t compressedSize;
} maptileset_t;

typedef struct {
    uint32_t x;
    uint32_t y;
} mapcheckpoint_t;

typedef struct {
    uint32_t pos[2];
    uint32_t entitytype;
    uint32_t entityid;
} mapspawn_t;

typedef struct {
	uint32_t ident;
	uint32_t version;
    lump_t lumps[NUMLUMPS];
} mapheader_t;

#define LEVEL_IDENT (('M'<<24)+('F'<<16)+('F'<<8)+'B')
#define LEVEL_VERSION 0

typedef struct {
    uint32_t ident;
    uint32_t version;
    
    mapheader_t map;
    tile2d_header_t tileset;
    tex2d_t texture;
} bmf_t;

#endif