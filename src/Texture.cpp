#include "gln.h"
#include "stb_image.h"
#include "Texture.h"

void CTexture::Clear(void)
{
    if (mFileBuf)
        FreeMemory(mFileBuf);
    if (mTexBuffer)
        FreeMemory(mTexBuffer);
    if (mId)
        glDeleteTextures(1, (const GLuint *)&mId);
}

void CTexture::Load(const std::string& path)
{
    stbi_uc *image;
    GLint min, mag;
    
    image = stbi_load(path.c_str(), (int *)&mWidth, (int *)&mHeight, (int *)&mChannels, 3);
    if (!image) {
        Printf("[CTexture::Load] failed to load texture file '%s', stbimage error message: %s", path.c_str(), stbi_failure_reason());
        return;
    }

    mTexBuffer = image;
    LoadFile(path.c_str(), (void **)&mFileBuf);

    Printf("[CTexture::Load] Initializing OpenGL texture object...");
    if (mName != GetFilename(path.c_str()))
        mName = GetFilename(path.c_str());
    
    switch (gameConfig->mTextureFiltering) {
    case 0: // Nearest
        min = GL_NEAREST;
        mag = GL_NEAREST;
        break;
    case 1: // Linear
        min = GL_LINEAR;
        mag = GL_LINEAR;
        break;
    case 2: // Bilinear
        min = GL_NEAREST;
        mag = GL_LINEAR;
        break;
    case 3: // Trilinear
        min = GL_LINEAR;
        mag = GL_NEAREST;
        break;
    };

    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, mChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image);
    Unbind();

    Printf("[CTexture::Load] done");
}
