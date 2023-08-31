#ifndef __EDITOR_MANAGER__
#define __EDITOR_MANAGER__

#pragma once

#include <EASTL/string_hash_map.h>

template<typename Tool>
class CEditorManager
{
public:
    CEditorManager(void) = default;
    ~CEditorManager() = default;

    virtual bool LoadList(void) = 0;
    virtual void ClearList(void) = 0;
    virtual void Draw(void) = 0;

    inline void *operator new(uint64_t size)
    { return Z_Malloc(size, TAG_STATIC, NULL, "op new"); }
    inline void *operator new[](uint64_t size)
    { return Z_Malloc(size, TAG_STATIC, NULL, "op new"); }
    inline void operator delete(void *ptr)
    { Z_Free(ptr); }
    inline void operator delete[](void *ptr)
    { Z_Free(ptr); }

    virtual bool HasWizard(void) const = 0;

    virtual inline int GetListSize(void) const
    { return listSize; }
    virtual inline int GetBits(void) const
    { return bits; }
    virtual inline const Tool* GetTool(const eastl::string& name) const
    {
        using iterator = typename eastl::string_hash_map<Tool>::const_iterator;
        iterator it = toolList.find(name);
        if (it != toolList.end())
            return &it->second;
        
        Printf("Bad lookup for '%s'", name.c_str());
        return NULL;
    }
    virtual inline Tool* GetTool(const eastl::string& name)
    {
        using iterator = typename eastl::string_hash_map<Tool>::iterator;
        iterator it = toolList.find(name);
        if (it != toolList.end())
            return &it->second;
        
        Printf("Bad lookup for '%s'", name.c_str());
        return NULL;
    }
protected:
    eastl::string_hash_map<Tool> toolList;
    int listSize;
    int bits;

};

#endif