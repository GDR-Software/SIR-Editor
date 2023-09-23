#ifndef __ENTITY__
#define __ENTITY__

#pragma once

typedef enum {
    ET_PLAYR = 0,
    ET_MOB,
    ET_BOT,
    ET_ITEM,
    ET_WEAPON,
    ET_AIR,

    NUMENTITIES
} entityType_t;

class CEntity
{
public:
    entityType_t mId;
    vec3_t pos[3];
    std::string mType;

    CEntity(void)
        : mId{ ET_AIR }, pos{ 0, 0, 0 }, mType{} { }
    ~CEntity() { }
};

#endif