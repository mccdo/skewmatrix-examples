// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#ifndef __MATERIAL_H__
#define __MATERIAL_H__ 1


#include <osgAudio/SOundState.h>


struct Material
{
    typedef enum {
        DEFAULT,
        CEMENT,
        JELLO,
        SILLY_PUTTY,
        FLUBBER,
        WOOD_DOOR
    } MaterialType;

    Material( MaterialType mat=DEFAULT )
      : _mat( mat )
    {}
    ~Material() {};

    bool operator<( const Material& mat ) const
    {
        return( ((int)_mat) < ((int)mat._mat) );
    }

    MaterialType _mat;
};


// __MATERIAL_H__
#endif
