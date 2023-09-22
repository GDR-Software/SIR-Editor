#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#pragma once

class CTexture
{
public:
    std::string mName;
    byte *mTexBuffer;
    char *mFileBuf;
    uint32_t mId;
    uint32_t mMinFilter;
    uint32_t mMagFilter;
    uint32_t mSamples;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mChannels;

    CTexture(const std::string& path)
        : mName{ GetFilename(path.c_str()) }, mTexBuffer{ NULL }, mFileBuf{ NULL }, mId{ 0 }, mMinFilter{ GL_NEAREST }, mMagFilter{ GL_NEAREST },
        mSamples{ 0 }, mWidth{ 0 }, mHeight{ 0 }
    { Load(path); }
    CTexture(void)
        : mName{ "None" }, mTexBuffer{ NULL }, mFileBuf{ NULL }, mId{ 0 }, mMinFilter{ GL_NEAREST }, mMagFilter{ GL_NEAREST },
        mSamples{ 0 }, mWidth{ 0 }, mHeight{ 0 }
    { }
    ~CTexture()
    { Clear(); }

    void Clear(void);
    void Load(const std::string& path);
    
    INLINE void Bind(void) const
    { glBindTexture(GL_TEXTURE_2D, mId); }
    INLINE void Unbind(void) const
    { glBindTexture(GL_TEXTURE_2D, 0); }
};

#endif