#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#pragma once

#include <glad/glad.h>

class CTexture
{
public:
    CTexture(void);
    ~CTexture();

    bool LoadImage(const byte *buffer, uint64_t buflen);
    bool LoadImage(const string_t& path);
    bool Load(const string_t& path);
    bool Load(const json& data);
    bool Save(json& data) const;
    bool Save(const string_t& path) const;
    void Clear(void);

    void ReInit(void); // reinitializes the texture's parameters for the gpu

    void SetParms(uint32_t min, uint32_t mag, uint32_t WrapS, uint32_t WrapT, uint32_t target)
    {
        mMinfilter = min;
        mMagfilter = mag;
        mWrapS = WrapS;
        mWrapT = WrapT;
        if (target == GL_TEXTURE_2D)
            mMultisampling = false;
        else if (target == GL_TEXTURE_2D_MULTISAMPLE)
            mMultisampling = true;
    }

    inline const string_t& GetName(void) const
    { return mName; }
    inline void SetName(const string_t& name)
    { mName = name; }

    bool Write(FILE *fp) const;
    bool Read(FILE *fp);
    void Read(const byte *buffer);

    inline uint32_t GetWidth(void) const
    { return mWidth; }
    inline uint32_t GetHeight(void) const
    { return mHeight; }
    inline uint32_t GetMagFilter(void) const
    { return mMagfilter; }
    inline uint32_t GetMinFilter(void) const
    { return mMinfilter; }
    inline uint32_t GetWrapS(void) const
    { return mWrapS; }
    inline uint32_t GetWrapT(void) const
    { return mWrapT; }
    inline uint32_t GetFormat(void) const
    { return mFormat; }
    inline bool IsMultisampled(void) const
    { return mMultisampling; }
    inline uint32_t GetChannels(void) const
    { return mChannels; }
    inline const vector_t<byte>& GetBuffer(void) const
    { return mImageBuffer; }
    inline vector_t<byte>& GetBuffer(void)
    { return mImageBuffer; }

    inline void Bind(uint32_t slot = 0) const
    {
        nglActiveTexture(GL_TEXTURE0+slot);
        nglBindTexture(mMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, mId);
    }
    inline void Unbind(void) const
    { nglBindTexture(mMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 0); }
private:
    vector_t<byte> mImageBuffer;
    vector_t<byte> mFileBuffer;

    string_t mName;
    uint32_t mMinfilter;
    uint32_t mMagfilter;
    uint32_t mWrapS;
    uint32_t mWrapT;
    uint32_t mFormat;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mChannels;
    uint32_t mId;
    bool mMultisampling;
};

#endif