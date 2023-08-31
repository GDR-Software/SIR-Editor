#include "Common.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"
#include <glad/glad.h>

CTexture::CTexture()
    : path{"None"}, width{0}, height{0}, id{0}
{
}

CTexture::~CTexture()
{
    Shutdown();
}

void CTexture::Shutdown(void)
{
    if (buffer)
        delete[] buffer;
    if (id != 0)
        glDeleteTextures(1, &id);
}

void CTexture::Load(const eastl::string& filename)
{
    stbi_uc *image;

    Printf("Loading texture file '%s'", filename.c_str());

    image = stbi_load(filename.c_str(), &width, &height, &channels, 4);
    if (!image) {
        Printf("Texture::Load: stbi_load(%s) failed, stbimage error: %s", filename.c_str(), stbi_failure_reason());
        return;
    }

    if (buffer)
        delete[] buffer;
    
    buffer = new uint8_t[width * height * channels];
    free(image);

    path = filename;
    Printf("Generating OpenGL texture...");

    if (!id)
        glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, buffer);

    glBindTexture(GL_TEXTURE_2D, 0);

    Printf("Done Loading Texture.");
}
