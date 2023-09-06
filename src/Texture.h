#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#pragma once

class CTexture : public CEditorTool
{
public:
    CTexture(void);
    virtual ~CTexture();

    virtual bool Load(const string_t& path);
    virtual bool Load(const json& data);
    virtual bool Save(json& data) const;
    virtual bool Save(const string_t& path) const;
    virtual void Clear(void);

    inline int GetWidth(void) const
    { return width; }
    inline int GetHeight(void) const
    { return height; }
private:
    eastl::vector<uint8_t> buffer;

    int width;
    int height;
    int channels;

    unsigned int id;
};

#endif