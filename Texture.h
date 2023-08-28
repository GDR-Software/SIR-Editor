#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#pragma once

#include <stb/stb_image.h>

class Texture
{
public:
    Texture();
    ~Texture();

    void Load(const eastl::string& filename);
    void Shutdown(void);
public:
    eastl::string path;
    eastl::vector<unsigned char> buffer;

    int width;
    int height;
    int channels;

    unsigned int id;
};

#endif