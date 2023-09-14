#include "gln.h"
#define STBI_MALLOC(size) GetMemory(size)
#define STBI_FREE(ptr) FreeMemory(ptr)
#define STBI_REALLOC(ptr,nsize) GetResizedMemory(ptr,nsize)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"

CTexture::CTexture(void)
{
    mMultisampling = false;
    mMinfilter = GL_LINEAR;
    mMagfilter = GL_NEAREST;
    mWrapS = GL_REPEAT;
    mWrapT = GL_REPEAT;
    mFormat = GL_RGBA8;
    mHeight = 0;
    mWidth = 0;
    mId = 0;
    mChannels = 0;
    mName = "None";
}

CTexture::~CTexture()
{
    Clear();
}

void CTexture::Clear(void)
{
    mImageBuffer.clear();
    if (mId != 0)
        nglDeleteTextures(1, (const GLuint *)&mId);
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

    texture["name"] = mName;
    texture["width"] = mWidth;
    texture["height"] = mHeight;
    texture["channels"] = mChannels;
    texture["format"] = FormatToString(mFormat);
    texture["mag"] = FilterToString(mMagfilter);
    texture["min"] = FilterToString(mMinfilter);
    texture["wrapS"] = WrapToString(mWrapS);
    texture["wrapT"] = WrapToString(mWrapT);

    data["texture"] = texture;

    return true;
}

bool CTexture::Load(const json& data)
{
    const json& texture = data.at("texture");

    mName = texture.at("name");
    mWidth = texture.at("width");
    mHeight = texture.at("height");
    mChannels = texture.at("channels");
    mFormat = StrToFormat(texture.at("format").get<string_t>().c_str());
    mMinfilter = StrToFilter(texture.at("min").get<string_t>().c_str());
    mMagfilter = StrToFilter(texture.at("mag").get<string_t>().c_str());
    mWrapS = StrToWrap(texture.at("wrapS").get<string_t>().c_str());
    mWrapT = StrToWrap(texture.at("wrapT").get<string_t>().c_str());

    return Load(mName);
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
    
    mFileBuffer.clear();
    mImageBuffer.clear();

    image = (const byte *)(tex + 1);

    if (!LoadImage(image, tex->fileSize))
        return;
}

void CTexture::ReInit(void)
{
    Printf("Reinitializing OpenGL texture parameters...");
    Bind();
    if (mMultisampling) {
        nglTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, mFormat, mWidth, mHeight, GL_FALSE);
    }
    else {
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mMinfilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mMagfilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapS);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapT);

        nglTexImage2D(GL_TEXTURE_2D, 0, mFormat, mWidth, mHeight, 0, mChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, mImageBuffer.data());
    }
    Unbind();
}

bool CTexture::LoadImage(const byte *buffer, uint64_t buflen)
{
    stbi_uc *image;

    image = stbi_load_from_memory((const stbi_uc *)buffer, buflen, (int *)&mWidth, (int *)&mHeight, (int *)&mChannels, 4);
    if (!image) {
        Printf("CTexture::Load: stbi_load_from_memory() failed, stbimage error: %s", stbi_failure_reason());
        return false;
    }
    mFileBuffer.clear();
    mFileBuffer.insert(mFileBuffer.end(), buffer, buffer + buflen);
    mImageBuffer.clear();
    mImageBuffer.insert(mImageBuffer.end(), image, image + (mWidth * mHeight * mChannels));
    
    Printf("Generating OpenGL texture...");
    if (!mId)
        nglGenTextures(1, (GLuint *)&mId);
    
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
    mFileBuffer.clear();
    mFileBuffer.resize(FileLength(fp));
    SafeRead(mFileBuffer.data(), mFileBuffer.size(), fp);
    fclose(fp);

    image = stbi_load(path.c_str(), &tmpwidth, &tmpheight, &tmpchannels, 4);
    if (!image) {
        Printf("CTexture::Load: stbi_load(%s) failed, stbimage error: %s", path.c_str(), stbi_failure_reason());
        return false;
    }

    mWidth = tmpwidth;
    mHeight = tmpheight;
    mChannels = tmpchannels;
    mImageBuffer.clear();
    mImageBuffer.insert(mImageBuffer.end(), image, image + (mWidth * mHeight * mChannels));
    FreeMemory(image);

    mName = path;
    Printf("Generating OpenGL texture...");
    if (!mId)
        nglGenTextures(1, (GLuint *)&mId);

    Bind();
    if (mMultisampling) {
        nglTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, mFormat, mWidth, mHeight, GL_FALSE);
    }
    else {
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mMinfilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mMagfilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mWrapS);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mWrapT);

        nglTexImage2D(GL_TEXTURE_2D, 0, mFormat, mWidth, mHeight, 0, mChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, mImageBuffer.data());
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

    outbuf = Compress((void *)mImageBuffer.data(), mImageBuffer.size(), &outlen);

    file.ident = TEX2D_IDENT;
    file.version = TEX2D_VERSION;
    file.channels = mChannels;
    file.format = mFormat;
    file.height = mHeight;
    file.width = mWidth;
    file.magfilter = mMagfilter;
    file.minfilter = mMinfilter;
    file.wrapS = mWrapS;
    file.wrapT = mWrapT;
    file.multisampling = mMultisampling;
    file.compression = parm_compression;
    file.compressedSize = outlen;
    file.fileSize = mFileBuffer.size();
    N_strncpyz(file.name, GetFilename(mName.c_str()), sizeof(file.name));

    SafeWrite(&file, sizeof(file), fp);
    SafeWrite(outbuf, outlen, fp);
    SafeWrite(mFileBuffer.data(), mFileBuffer.size(), fp); // write the raw image data

    FreeMemory(outbuf);

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

    cBuf = (char *)GetMemory(file.compressedSize);
    SafeRead(cBuf, file.compressedSize, fp);

    inbuf = Decompress(cBuf, file.compressedSize, &inlen, file.compression);
    
    mImageBuffer.clear();
    mImageBuffer.insert(mImageBuffer.end(), inbuf, inbuf + inlen);
    mFileBuffer.clear();
    mFileBuffer.resize(file.fileSize);

    SafeRead(mFileBuffer.data(), mFileBuffer.size(), fp);

    FreeMemory(cBuf);

    return true;
}
