#ifndef __EDITOR_TOOL__
#define __EDITOR_TOOL__

#pragma once

class CEditorTool
{
public:
    CEditorTool(void);
    ~CEditorTool() = default;

    virtual bool Load(const std::string& path) = 0;
    virtual bool Load(const json& data) = 0;
    virtual bool Save(const std::string& path) const = 0;
    virtual bool Save(json& data) const = 0;
    virtual void Clear(void) = 0;

    inline void *operator new(uint64_t size)
    { return Z_Malloc(size, TAG_STATIC, NULL, "op new"); }
    inline void *operator new[](uint64_t size)
    { return Z_Malloc(size, TAG_STATIC, NULL, "op new"); }
    inline void operator delete(void *ptr)
    { Z_Free(ptr); }
    inline void operator delete[](void *ptr)
    { Z_Free(ptr); }

    virtual inline void SetName(const eastl::string& _name)
    { name = _name; }
    virtual inline void SetModified(bool _modified)
    { modified = _modified; }
    virtual inline const eastl::string& GetName(void) const
    { return name; }
    virtual bool GetModified(void) const
    { return modified; }
protected:
    eastl::string name;
    bool modified;
};

#endif