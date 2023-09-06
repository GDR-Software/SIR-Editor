#include "Common.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include <glad/glad.h>

CTexture::CTexture(void)
{
    height = 0;
    width = 0;
    id = 0;
    name = "None";
}

CTexture::~CTexture()
{
    Clear();
}

void CTexture::Clear(void)
{
    buffer.clear();
    if (id != 0)
        glDeleteTextures(1, &id);
}

bool CTexture::Save(const string_t& path) const
{
    return true;
}

bool CTexture::Save(json& data) const
{
    json texture;

    texture["name"] = name;
    texture["width"] = width;
    texture["height"] = height;
    texture["channels"] = channels;

    data["texture"] = texture;

    return true;
}

bool CTexture::Load(const json& data)
{
    const json& texture = data.at("texture");

    name = texture.at("name");
    width = texture.at("width");
    height = texture.at("height");
    channels = texture.at("channels");

    return Load(name);
}


bool CTexture::Load(const string_t& path)
{
    stbi_uc *image;
    int tmpwidth, tmpheight, tmpchannels;

    Printf("Loading texture file '%s'", path.c_str());

    image = stbi_load(path.c_str(), &tmpwidth, &tmpheight, &tmpchannels, 4);
    if (!image) {
        Printf("Texture::Load: stbi_load(%s) failed, stbimage error: %s", path.c_str(), stbi_failure_reason());
        return false;
    }

    width = tmpwidth;
    height = tmpheight;
    channels = tmpchannels;
    buffer.clear();
    buffer.insert(buffer.end(), image, image + (width * height * channels));

    name = path;
    Printf("Generating OpenGL texture...");

    if (!id)
        glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

    glBindTexture(GL_TEXTURE_2D, 0);

    Printf("Done Loading Texture.");

    return true;
}
