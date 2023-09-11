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

void CTexture::Read(const byte *buffer)
{
    const tex2d_t *tex;
    const byte *image;

    tex = (const tex2d_t *)buffer;

    if (tex->ident != TEX2D_IDENT)
        return;
    if (tex->version != TEX2D_VERSION)
        return;
    
    filebuffer.clear();
    imagebuffer.clear();

    image = (const byte *)(tex + 1);

    if (!LoadImage(image, tex->fileSize))
        return;
}

void CTexture::ReInit(void)
{
    Printf("Reinitializing OpenGL texture parameters...");
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
}

bool CTexture::LoadImage(const byte *buffer, uint64_t buflen)
{
    stbi_uc *image;

    image = stbi_load_from_memory((const stbi_uc *)buffer, buflen, (int *)&width, (int *)&height, (int *)&channels, 4);
    if (!image) {
        Printf("CTexture::Load: stbi_load_from_memory() failed, stbimage error: %s", stbi_failure_reason());
        return false;
    }
    
    Printf("Generating OpenGL texture...");
    if (!id)
        glGenTextures(1, (GLuint *)&id);
    
    ReInit();

    return true;
}

bool CTexture::LoadImage(const string_t& path)
{
    stbi_uc *image;
    int tmpwidth, tmpheight, tmpchannels;
    FILE *fp;

    Printf("Loading texture file '%s'", path.c_str());

    if (!FileExists(path.c_str())) {
        Printf("CTexture::Load: bad texture path '%s', file doesn't exist", path.c_str());
        return false;
    }

    fp = SafeOpenRead(path.c_str());
    filebuffer.resize(FileLength(fp));
    SafeRead(filebuffer.data(), filebuffer.size(), fp);
    fclose(fp);

    image = stbi_load(path.c_str(), &tmpwidth, &tmpheight, &tmpchannels, 4);
    if (!image) {
        Printf("CTexture::Load: stbi_load(%s) failed, stbimage error: %s", path.c_str(), stbi_failure_reason());
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
    tex2d_t file;
    char *outbuf;
    uint64_t outlen;

    memset(&file, 0, sizeof(file));

    outbuf = Compress((void *)imagebuffer.data(), imagebuffer.size(), &outlen);

    file.ident = TEX2D_IDENT;
    file.version = TEX2D_VERSION;
    file.channels = channels;
    file.format = format;
    file.height = height;
    file.width = width;
    file.magfilter = magfilter;
    file.minfilter = minfilter;
    file.wrapS = wrapS;
    file.wrapT = wrapT;
    file.multisampling = multisampling;
    file.compression = parm_compression;
    file.compressedSize = outlen;
    file.fileSize = filebuffer.size();
    N_strncpyz(file.name, GetFilename(name.c_str()), sizeof(file.name));

    SafeWrite(&file, sizeof(file), fp);
    SafeWrite(outbuf, outlen, fp);
    SafeWrite(filebuffer.data(), filebuffer.size(), fp); // write the raw image data

    Free(outbuf);

    return true;
}

bool CTexture::Read(FILE *fp)
{
    tex2d_t file;
    char *inbuf, *cBuf;
    uint64_t inlen;

    memset(&file, 0, sizeof(file));

    SafeRead(&file, sizeof(file), fp);

    if (file.ident != TEX2D_IDENT) {
        Printf("ERROR: bad tex2d header indentifier: %lu", file.ident);
        return false;
    }
    if (file.version != TEX2D_VERSION) {
        Printf("ERROR: bad tex2d header version: %lu", file.version);
        return false;
    }

    cBuf = (char *)Malloc(file.compressedSize);
    SafeRead(cBuf, file.compressedSize, fp);

    inbuf = Decompress(cBuf, file.compressedSize, &inlen, file.compression);
    
    imagebuffer.clear();
    imagebuffer.insert(imagebuffer.end(), inbuf, inbuf + inlen);
    filebuffer.clear();
    filebuffer.resize(file.fileSize);

    SafeRead(filebuffer.data(), filebuffer.size(), fp);

    Free(cBuf);

    return true;
}
