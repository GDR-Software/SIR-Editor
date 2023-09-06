#ifndef __EDITOR_MANAGER__
#define __EDITOR_MANAGER__

#pragma once

template<class Tool>
using tool_hash_table = string_hash_t<object_ptr_t<Tool>>;

class CEditorManager
{
public:
    CEditorManager(void);
    virtual ~CEditorManager();

    virtual bool LoadList(void);
    virtual void ClearList(void);
    virtual void DrawRecent(void) = 0;
    virtual void DrawWizard(const string_t& menuTitle) = 0;
    virtual void DrawWizard(void) = 0;

    virtual bool HasWizard(void) const = 0;
    virtual bool HasOpenRecent(void) const = 0;
    virtual const char *GetManagerName(void) const = 0;
    virtual const char *GetWizardName(void) const = 0;
};

#define IMPL_EDITOR_MANAGER(manager,tool) \
public: \
    virtual bool LoadList(void); \
    virtual void ClearList(void); \
    virtual void DrawRecent(void) override; \
    inline int GetBits(void) const { return bits; } \
    inline const tool_hash_table<tool>& GetList(void) const \
    { return toolList; } \
    inline tool_hash_table<tool>& GetList(void) \
    { return toolList; } \
    inline int GetListSize(void) const { return listSize; } \
    inline const object_ptr_t<tool>& GetTool(const string_t& name) const \
    { \
        using iterator = tool_hash_table<tool>::const_iterator; \
        iterator it = toolList.find(name.c_str()); \
        if (it != toolList.cend()) { \
            return it->second; \
        } \
        Error("Bad lookup for '%s'", name.c_str()); \
    } \
    inline object_ptr_t<tool>& GetTool(const string_t& name) \
    { \
        using iterator = tool_hash_table<tool>::iterator; \
        iterator it = toolList.find(name.c_str()); \
        if (it != toolList.cend()) { \
            return it->second; \
        } \
        Error("Bad lookup for '%s'", name.c_str()); \
    } \
private: \
    tool_hash_table<tool> toolList; \
    int listSize; \
    int bits;

#define IMPL_SINGLETON(manager,tool) \
private: \
    object_ptr_t<tool> curTool; \
public: \
    inline const string_t& GetCurrentName(void) const \
    { return curTool->GetName(); } \
    inline void SetCurrentName(const string_t& name) \
    { curTool->SetName(name); } \
    inline bool GetCurrentModified(void) const \
    { return curTool->GetModified(); } \
    inline void SetCurrentModified(bool _modified) \
    { curTool->SetModified(_modified); } \
    inline const object_ptr_t<tool>& GetCurrent(void) const \
    { return curTool; } \
    inline object_ptr_t<tool>& GetCurrent(void) \
    { return curTool; } \
    inline bool LoadCurrent(const string_t& path) \
    { return curTool->Load(path); } \
    inline bool LoadCurrent(const json& data) \
    { return curTool->Load(data); } \
    inline bool SaveCurrent(const string_t& path) \
    { return curTool->Save(path); } \
    inline bool SaveCurrent(json& data) \
    { return curTool->Save(data); } \
    inline void ClearCurrent(void) \
    { curTool->Clear(); }

#define IMPL_WIZARD_DIMS(array,width,height) \
    union { \
        int array[2]; \
        struct { \
            int width; \
            int height; \
        }; \
    };

#endif