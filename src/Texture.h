#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#pragma once

#include <glad/glad.h>

class CTexture : public CEditorTool
{
public:
    CTexture(void);
    virtual ~CTexture();

    bool LoadImage(const string_t& path);
    virtual bool Load(const string_t& path);
    virtual bool Load(const json& data);
    virtual bool Save(json& data) const;
    virtual bool Save(const string_t& path) const;
    virtual void Clear(void);

    void SetParms(uint32_t min, uint32_t mag, uint32_t WrapS, uint32_t WrapT, uint32_t target)
    {
        minfilter = min;
        magfilter = magfilter;
        wrapS = WrapS;
        wrapT = WrapT;
        if (target == GL_TEXTURE_2D)
            multisampling = false;
        else if (target == GL_TEXTURE_2D_MULTISAMPLE)
            multisampling = true;
    }

    bool Write(FILE *fp) const;
    bool Read(FILE *fp);

    inline uint32_t GetWidth(void) const
    { return width; }
    inline uint32_t GetHeight(void) const
    { return height; }
    inline uint32_t GetMagFilter(void) const
    { return magfilter; }
    inline uint32_t GetMinFilter(void) const
    { return minfilter; }
    inline uint32_t GetWrapS(void) const
    { return wrapS; }
    inline uint32_t GetWrapT(void) const
    { return wrapT; }
    inline uint32_t GetChannels(void) const
    { return channels; }
    inline const vector_t<byte>& GetBuffer(void) const
    { return imagebuffer; }
    inline vector_t<byte>& GetBuffer(void)
    { return imagebuffer; }

    inline void Bind(uint32_t slot = 0) const
    {
        glActiveTexture(GL_TEXTURE0+slot);
        glBindTexture(multisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, id);
    }
    inline void Unbind(void) const
    { glBindTexture(multisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, 0); }
private:
    vector_t<byte> imagebuffer;
    vector_t<byte> filebuffer;

    uint32_t minfilter;
    uint32_t magfilter;
    uint32_t wrapS;
    uint32_t wrapT;
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t id;
    bool multisampling;
};

#endif