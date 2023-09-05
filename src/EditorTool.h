#ifndef __EDITOR_TOOL__
#define __EDITOR_TOOL__

#pragma once

class CEditorTool
{
public:
    CEditorTool(void);
    virtual ~CEditorTool();

    virtual bool Load(const string_t& path);
    virtual bool Load(const json& data);
    virtual bool Save(void) const;
    virtual bool Save(const string_t& path) const;
    virtual bool Save(json& data) const;
    virtual void Clear(void);

    virtual void SetPath(const path_t& _path);
    virtual void SetName(const string_t& _name);
    virtual inline void SetModified(bool _modified)
    { modified = _modified; }
    virtual inline const string_t& GetName(void) const
    { return name; }
    virtual inline const path_t& GetPath(void) const
    { return path; }
    virtual bool GetModified(void) const
    { return modified; }
protected:
    string_t name;
    path_t path; // absolute path
    mutable bool modified;
};

#endif