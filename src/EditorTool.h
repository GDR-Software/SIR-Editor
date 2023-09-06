#ifndef __EDITOR_TOOL__
#define __EDITOR_TOOL__

#pragma once

class CEditorTool
{
public:
    CEditorTool(void);
    virtual ~CEditorTool();

    virtual bool Load(const string_t& path)
    { Error("CEditorTool::%s: called directly", __func__); return false; }
    virtual bool Load(const json& data)
    { Error("CEditorTool::%s: called directly", __func__); return false; }
    virtual bool Save(void) const
    { Error("CEditorTool::%s: called directly", __func__); return false; }
    virtual bool Save(const string_t& path) const\
    { Error("CEditorTool::%s: called directly", __func__); return false; }
    virtual bool Save(json& data) const
    { Error("CEditorTool::%s: called directly", __func__); return false; }
    virtual void Clear(void)
    { Error("CEditorTool::%s: called directly", __func__); }

    virtual inline void SetName(const string_t& _name)
    { name = _name; }
    virtual inline void SetModified(bool _modified)
    { modified = _modified; }
    virtual inline const string_t& GetName(void) const
    { return name; }
    virtual bool GetModified(void) const
    { return modified; }
protected:
    string_t name;
    mutable bool modified;
};

#endif