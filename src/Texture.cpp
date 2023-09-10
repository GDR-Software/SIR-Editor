#include "Common.hpp"
#define STBI_MALLOC(size) Malloc(size)
#define STBI_FREE(ptr) Free(ptr)
#ifdef USE_ZONE
#define STBI_REALLOC(ptr,nsize) Z_Realloc(ptr,nsize,TAG_STATIC,NULL,"zalloc")
#else
void *Mem_Realloc(void *ptr, uint32_t nsize);
#define STBI_REALLOC(ptr,nsize) Mem_Realloc(ptr,nsize)
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"

CTexture::CTexture(void)
{
    multisampling = false;
    minfilter = GL_LINEAR;
    magfilter = GL_NEAREST;
    wrapS = GL_REPEAT;
    wrapT = GL_REPEAT;
    format = GL_RGBA8;
    height = 0;
    width = 0;
    id = 0;
    channels = 0;
    name = "None";
}

CTexture::~CTexture()
{
    Clear();
}

void CTexture::Clear(void)
{
    imagebuffer.clear();
    if (id != 0)
        glDeleteTextures(1, (const GLuint *)&id);
}

bool CTexture::Save(const string_t& path) const
{
    FILE *fp;

    fp = SafeOpenWrite(path.c_str());
    if (!Write(fp))
        return false;

    fclose(fp);

    return true;
}

bool CTexture::Save(json& data) const
{
    json texture;

    texture["name"] = name;
    texture["width"] = width;
    texture["height"] = height;
    texture["channels"] = channels;
    texture["format"] = FormatToString(format);
    texture["mag"] = FilterToString(magfilter);
    texture["min"] = FilterToString(minfilter);
    texture["wrapS"] = WrapToString(wrapS);
    texture["wrapT"] = WrapToString(wrapT);

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
    format = StrToFormat(texture.at("format").get<string_t>().c_str());
    minfilter = StrToFilter(texture.at("min").get<string_t>().c_str());
    magfilter = StrToFilter(texture.at("mag").get<string_t>().c_str());
    wrapS = StrToWrap(texture.at("wrapS").get<string_t>().c_str());
    wrapT = StrToWrap(texture.at("wrapT").get<string_t>().c_str());

    return Load(name);
}

bool CTexture::Load(const string_t& path)
{
    FILE *fp;

    fp = SafeOpenRead(path.c_str());
    if (!Read(fp))
        return false;
    
    fclose(fp);
    return true;
}

bool CTexture::LoadImage(const string_t& path)
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
    imagebuffer.clear();
    imagebuffer.insert(imagebuffer.end(), image, image + (width * height * channels));
    Free(image);

    name = path;
    Printf("Generating OpenGL texture...");
    if (!id)
        glGenTextures(1, (GLuint *)&id);

    Bind();
    if (multisampling) {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, format, width, height, GL_FALSE);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, imagebuffer.data());
    }
    Unbind();

    Printf("Done Loading Texture.");

    return true;
}

bool CTexture::Write(FILE *fp) const
{
    tex2d_header_t header;
    tex2d_info_t info;
    uint64_t pos;
    char *outbuf;
    uint64_t outlen;

    memset(&header, 0, sizeof(header));

    outbuf = Compress((void *)imagebuffer.data(), imagebuffer.size(), &outlen);

    header.magic = TEXTURE_MAGIC;
    header.version = TEXTURE_VERSION;

    info.channels = channels;
    info.format = format;
    info.height = height;
    info.width = width;
    info.magfilter = magfilter;
    info.minfilter = minfilter;
    info.wrapS = wrapS;
    info.wrapT = wrapT;
    info.multisampling = multisampling;
    info.compression = parm_compression;
    info.compressedSize = outlen;

    pos = ftell(fp);

    // overwritten later
    SafeWrite(&header, sizeof(header), fp);

    AddLump(&info, sizeof(info), header.lumps, TEX_LUMP_INFO, fp);
    AddLump(outbuf, info.compressedSize, header.lumps, TEX_LUMP_BUF, fp);

    fseek(fp, pos, SEEK_SET);
    SafeWrite(&header, sizeof(header), fp);

    Free(outbuf);

    return true;
}

bool CTexture::Read(FILE *fp)
{
    tex2d_header_t header;
    tex2d_info_t info;
    uint64_t pos;
    char *inbuf, *cBuf;
    uint64_t inlen;

    memset(&header, 0, sizeof(header));

    SafeRead(&header, sizeof(header), fp);

    if (header.magic != TEXTURE_MAGIC) {
        Printf("ERROR: bad tex2d header magic: %lu", header.magic);
        return false;
    }
    if (header.version != TEXTURE_VERSION) {
        Printf("ERROR: bad tex2d header version: %lu", header.version);
        return false;
    }

    CopyLump((void *)&info, sizeof(tex2d_info_t), header.lumps, TEX_LUMP_INFO, fp);
    CopyLump((void **)&cBuf, sizeof(byte), header.lumps, TEX_LUMP_BUF, fp);

    inbuf = Decompress(cBuf, info.compressedSize, &inlen, info.compression);
    imagebuffer.clear();
    imagebuffer.insert(imagebuffer.end(), inbuf, inbuf + inlen);
    Free(cBuf);

    return true;
}
