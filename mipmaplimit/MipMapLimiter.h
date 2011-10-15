/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * osgWorks is (C) Copyright 2009-2011 by Kenneth Mark Bryden
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef __OSGWTOOLS_MIP_MAP_LIMITER_H__
#define __OSGWTOOLS_MIP_MAP_LIMITER_H__ 1


#include <osg/NodeVisitor>


// Forward.
namespace osg {
    class StateAttribute;
    class Texture1D;
    class Texture2D;
    class Texture3D;
    class TextureCubeMap;
    class TextureRectangle;
}


namespace osgwTools
{


/** \class MipMapLimiter MipMapLimiter.h <osgwTools/MipMapLimiter.h>
\brief A NodeVisitor that deletes mipmap levels above a specified size.

This class modifies all Texture StateAttributes in a scene graph so that
they do not produce mipmaps with levels exceeding a specified dimension.
By default, the maximum mipmap dimension is 1024. This can be changes in
the MipMapLimiter() constructor, or with the setMaxDimension() method.

If a given Texture contains an Image with loaded data, mipmap levels
exceeding the specified dimension are discarded, meaning that the Texture
Image will no longer reference them.
*/
class MipMapLimiter : public osg::NodeVisitor
{
public:
    MipMapLimiter( unsigned int contextID, osg::NodeVisitor::TraversalMode mode=osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );

    /** \brief Specifies the number of levels to remove, or the max dimension of a texture.

    MipMapLimiter interprets \c limitValue differently based on the value of LimitMode. See
    setLimitMode(). If LimitMade is REMOVE_LEVELS, then \c limitValue represents the number
    of mipmap levels to discard, starting at the base level, for every texture object encountered.
    If LimitMode is MAX_DIMENSION, then \c limitValue represents the maximum texture dimension
    allowed, and all levels with a dimension in excess of \c limitValue are discarded.

    \param limitValue Default is 1 level, in conjunction with the default LimitMode of REMOVE_LEVELS.

    \see setLimitModeAndValue(). */
    void setLimitValue( unsigned int limitValue ) { _limitValue = limitValue; }
    unsigned int getLimitValue() const { return( _limitValue ); }

    typedef enum {
        REMOVE_LEVELS,
        MAX_DIMENSION
    } LimitMode;

    /** Specifies the algorithm MipMapLimiter uses to reduce texture size.

    The default value is REMOVE_LEVELS, in which case the limit value (see setLimitValue())
    specified the number of levels to remove, starting with the base level. If the LimitMode
    is set to MAX_DIMENSION, then the limit value specifies the maximum allowed texture dimension,
    and all levels with a dimension in excess of the limit value are discarded.

    \see setLimitModeAndValue(). */
    void setLimitMode( LimitMode limitMode ) { _limitMode = limitMode; }
    LimitMode getLimitMode() const { return( _limitMode ); }

    /** Convenience routine to set both the LimitMode and limit value from one function. */
    void setLimitModeAndValue( LimitMode limitMode, unsigned int limitValue )
    {
        _limitMode = limitMode;
        _limitValue = limitValue;
    }

    /** Convenience routine to set wether textures are written out to disk instead of the ive file. */
    void setTextureIOFlag( bool enable ){ _outputTextures = enable; }
    
    virtual void apply( osg::Node& node );
    virtual void apply( osg::Geode& node );

    unsigned int totalAllTextures;
    unsigned int totalTexturesExceedingMaxDimension;
    unsigned int totalUnsupported;

protected:
    unsigned int _contextID;

    void apply( osg::StateSet* stateSet );
    void apply( osg::Texture1D* tex );
    void apply( osg::Texture2D* tex );
    void apply( osg::Texture3D* tex );
    void apply( osg::TextureCubeMap* tex );
    void apply( osg::TextureRectangle* tex );

    unsigned int clampPowerOf2( const unsigned int in );

    unsigned int _limitValue;
    LimitMode _limitMode;
    
    bool _outputTextures;
};

// osgwTools
}


// __OSGWTOOLS_MIP_MAP_LIMITER_H__
#endif
