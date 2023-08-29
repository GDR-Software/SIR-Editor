#ifndef __EDITOR_MANAGER__
#define __EDITOR_MANAGER__

#pragma once

template<typename Tool>
class CEditorManager
{
public:
    CEditorManager(void) = default;
    ~CEditorManager() = default;

    virtual bool LoadList(void) = 0;
    virtual void Clear(void) = 0;

    virtual inline int GetListSize(void) const
    { return listSize; }
    virtual inline const Tool* GetTool(const eastl::string& name) const
    {
        using iterator = typename eastl::unordered_map<eastl::string, Tool>::const_iterator;
        iterator it = toolList.find(name);
        if (it != toolList.end())
            return &it->second;
        
        Printf("Bad lookup for '%s'", name.c_str());
        return NULL;
    }
    virtual inline Tool* GetTool(const eastl::string& name)
    {
        using iterator = typename eastl::unordered_map<eastl::string, Tool::iterator;
        iterator it = toolList.find(name);
        if (it != toolList.end())
            return &it->second;
        
        Printf("Bad lookup for '%s'", name.c_str());
        return NULL;
    }
private:
    eastl::unordered_map<eastl::string, Tool> toolList;
    int listSize;
};

#endif