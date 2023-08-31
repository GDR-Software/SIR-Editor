#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#pragma once

#include <stb/stb_image.h>

class CTexture
{
public:
    CTexture();
    ~CTexture();

    void Load(const eastl::string& filename);
    void Shutdown(void);
public:
    eastl::string path;
    uint8_t *buffer;

    int width;
    int height;
    int channels;

    unsigned int id;
};

#endif