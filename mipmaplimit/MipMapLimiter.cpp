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

#include "MipMapLimiter.h"
#include <osg/Geode>
#include <osg/StateSet>
#include <osg/Texture>
#include <osg/Texture1d>
#include <osg/Texture2d>
#include <osg/Texture3d>
#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osg/Texture2dArray>
#include <osg/Image>
#include <osg/Math>



osg::Image* readImageFromCurrentTexture( unsigned int contextID, unsigned int levelToRead, GLenum type=GL_UNSIGNED_BYTE )
{
    osg::ref_ptr< osg::Image > image = new osg::Image;
    if( !image.valid() )
    {
        osg::notify( osg::WARN ) << "Warning: osgwTools::readImageFromCurrentTexture(): out of memory, no image read." << std::endl;
        return( NULL );
    }

    const osg::Texture::Extensions* extensions = osg::Texture::getExtensions( contextID, true );
    const osg::Texture3D::Extensions* extensions3D = osg::Texture3D::getExtensions( contextID, true );
    const osg::Texture2DArray::Extensions* extensions2DArray = osg::Texture2DArray::getExtensions( contextID, true );


    GLboolean binding1D, binding2D, binding3D, binding2DArray;
    glGetBooleanv( GL_TEXTURE_BINDING_1D, &binding1D );
    glGetBooleanv( GL_TEXTURE_BINDING_2D, &binding2D );
    glGetBooleanv( GL_TEXTURE_BINDING_3D, &binding3D );
    if( extensions2DArray->isTexture2DArraySupported() )
        glGetBooleanv( GL_TEXTURE_BINDING_2D_ARRAY_EXT, &binding2DArray );
    else
        binding2DArray = GL_FALSE;

    GLenum textureMode = binding1D ? GL_TEXTURE_1D : binding2D ? GL_TEXTURE_2D : binding3D ? GL_TEXTURE_3D : binding2DArray ? GL_TEXTURE_2D_ARRAY_EXT : 0;
    if( textureMode == 0 )
        return( NULL );


    GLint texW, texH, texD;
    glGetTexLevelParameteriv( textureMode, levelToRead, GL_TEXTURE_WIDTH, &texW );
    glGetTexLevelParameteriv( textureMode, levelToRead, GL_TEXTURE_HEIGHT, &texH );
    glGetTexLevelParameteriv( textureMode, levelToRead, GL_TEXTURE_DEPTH, &texD );
    if( ( texW == 0 ) || ( texH == 0 ) || ( texD == 0 ) )
    {
        osg::notify( osg::DEBUG_FP ) << "osgwTools::readImageFromCurrentTexture(): Requested mipmap level not available." << std::endl;
        return( NULL );
    }

    GLint internalFormat;
    glGetTexLevelParameteriv( textureMode, levelToRead, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );

    GLint packing;
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &packing );
    glPixelStorei( GL_PACK_ALIGNMENT, packing );


    GLint compressed = 0;
    if( textureMode == GL_TEXTURE_2D )
    {
        if( extensions->isCompressedTexImage2DSupported() )
            glGetTexLevelParameteriv( textureMode, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed );
    }
    else if( textureMode == GL_TEXTURE_3D )
    {
        if( extensions3D->isCompressedTexImage3DSupported() )
            glGetTexLevelParameteriv( textureMode, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed );
    }
    else if( textureMode == GL_TEXTURE_2D_ARRAY_EXT )
    {
        if( extensions2DArray->isCompressedTexImage3DSupported() )
            glGetTexLevelParameteriv( textureMode, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed );
    }


    GLenum pixelFormat;
    unsigned char* data( NULL );
    if( compressed == GL_TRUE )
    {
        pixelFormat = internalFormat;

        GLint totalSize;
        glGetTexLevelParameteriv( textureMode, levelToRead, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &totalSize );

        data = new unsigned char[ totalSize ];
        if( data == NULL )
        {
            osg::notify( osg::WARN ) << "Warning: osgwTools::readImageFromCurrentTexture(): out of memory, no image read." << std::endl;
            return( NULL );
        }
        extensions->glGetCompressedTexImage( textureMode, levelToRead, data );
    }
    else
    {
        pixelFormat = osg::Image::computePixelFormat( internalFormat );

        const GLint totalSize = osg::Image::computeRowWidthInBytes( texW, internalFormat, type, packing ) * texH * texD;

        data = new unsigned char[ totalSize ];
        if( data == NULL )
        {
            osg::notify( osg::WARN ) << "Warning: osgwTools::readImageFromCurrentTexture(): out of memory, no image read." << std::endl;
            return( NULL );
        }
        glGetTexImage( textureMode, levelToRead, pixelFormat, type, data );
    }
    image->setImage( texW, texH, texD, internalFormat, pixelFormat, type,
        data, osg::Image::USE_NEW_DELETE, packing );

    GLenum err = glGetError();
    if( err != GL_NO_ERROR )
        osg::notify( osg::WARN ) << "ERROR" << std::endl;

    return( image.release() );
}





namespace osgwTools
{


MipMapLimiter::MipMapLimiter( unsigned int contextID, osg::NodeVisitor::TraversalMode mode )
  : osg::NodeVisitor( mode ),
    _contextID( contextID ),
    _limitValue( 1 ),
    _limitMode( REMOVE_LEVELS )
{
    totalAllTextures = 0;
    totalTexturesExceedingMaxDimension = 0;
    totalUnsupported = 0;
}

void MipMapLimiter::apply( osg::Node& node )
{
    if( node.getStateSet() != NULL )
        apply( node.getStateSet() );

    traverse( node );
}
void MipMapLimiter::apply( osg::Geode& node )
{
    if( node.getStateSet() != NULL )
        apply( node.getStateSet() );

    unsigned int idx;
    for( idx=0; idx < node.getNumDrawables(); idx++ )
    {
        osg::Drawable* draw = node.getDrawable( idx );
        if( draw->getStateSet() != NULL )
            apply( draw->getStateSet() );
    }

    traverse( node );
}

void MipMapLimiter::apply( osg::StateSet* stateSet )
{
    unsigned int idx;
    for( idx=0; idx<16; idx++ )
    {
        osg::Texture* tex = static_cast< osg::Texture* >(
            stateSet->getTextureAttribute( idx, osg::StateAttribute::TEXTURE ) );
        if( tex == NULL )
            continue;
        totalAllTextures++;

        tex->setUnRefImageDataAfterApply( true );

        // Bind the texture object.
        osg::Texture::TextureObject* to = tex->getTextureObject( _contextID );
        if( to == NULL )
        {
            totalUnsupported++;
            return;
        }
        to->bind();

        osg::Texture1D* t1d = dynamic_cast< osg::Texture1D* >( tex );
        if( t1d != NULL )
        {
            apply( t1d );
            continue;
        }
        osg::Texture2D* t2d = dynamic_cast< osg::Texture2D* >( tex );
        if( t2d != NULL )
        {
            apply( t2d );
            continue;
        }
        osg::Texture3D* t3d = dynamic_cast< osg::Texture3D* >( tex );
        if( t3d != NULL )
        {
            apply( t3d );
            continue;
        }
        osg::TextureCubeMap* tcm = dynamic_cast< osg::TextureCubeMap* >( tex );
        if( tcm != NULL )
        {
            apply( tcm );
            continue;
        }
        osg::TextureRectangle* tr = dynamic_cast< osg::TextureRectangle* >( tex );
        if( tr != NULL )
        {
            apply( tr );
            continue;
        }
    }
}

void MipMapLimiter::apply( osg::Texture1D* tex )
{
    //
    // Compute the number of levels in the mipmap pyramid we wish to discard.
    //

    int levelsToRemove( 0 );

    int w = tex->getTextureWidth();
    if( _limitMode == MAX_DIMENSION )
    {
        int origW( w );
        w = clampPowerOf2( osg::minimum< int >( w, (int)_limitValue ) );
        int wLevels;
        for( wLevels=0; origW > w; wLevels++, origW >>= 1 ) {}
        levelsToRemove = wLevels;
    }
    else if( _limitMode == REMOVE_LEVELS )
    {
        levelsToRemove = _limitValue;
        w >>= _limitValue;
        w = osg::maximum< int >( w, 1 );
    }
    if( w == tex->getTextureWidth() )
    {
        // Nothing to do.
        return;
    }
    totalTexturesExceedingMaxDimension++;


    //
    // Get the mipmap level from the hardware.
    //

    osg::Image* newImage = readImageFromCurrentTexture( _contextID, levelsToRemove );
    if( newImage == NULL )
    {
        // readImageFromCurrentTexture() displays a message; set notify DEBUG_FP to see it.
        return;
    }

    tex->setImage( newImage );
    tex->setUseHardwareMipMapGeneration( true );
}

void MipMapLimiter::apply( osg::Texture2D* tex )
{
    //
    // Compute the number of levels in the mipmap pyramid we wish to discard.
    //

    int levelsToRemove( 0 );

    int w = tex->getTextureWidth();
    int h = tex->getTextureHeight();
    if( _limitMode == MAX_DIMENSION )
    {
        int origW( w );
        int origH( h );
        w = clampPowerOf2( osg::minimum< int >( w, (int)_limitValue ) );
        h = clampPowerOf2( osg::minimum< int >( h, (int)_limitValue ) );
        int wLevels, hLevels;
        for( wLevels=0; origW > w; wLevels++, origW >>= 1 ) {}
        for( hLevels=0; origH > h; hLevels++, origH >>= 1 ) {}
        levelsToRemove = osg::maximum< int >( wLevels, hLevels );
    }
    else if( _limitMode == REMOVE_LEVELS )
    {
        levelsToRemove = _limitValue;
        w >>= _limitValue;
        h >>= _limitValue;
        w = osg::maximum< int >( w, 1 );
        h = osg::maximum< int >( h, 1 );
    }
    if( ( w == tex->getTextureWidth() ) &&
        ( h == tex->getTextureHeight() ) )
    {
        // Nothing to do.
        return;
    }
    totalTexturesExceedingMaxDimension++;


    //
    // Get the mipmap level from the hardware.
    //

    osg::Image* newImage = readImageFromCurrentTexture( _contextID, levelsToRemove );
    if( newImage == NULL )
    {
        // readImageFromCurrentTexture() displays a message; set notify DEBUG_FP to see it.
        return;
    }

    tex->setImage( newImage );
    tex->setUseHardwareMipMapGeneration( true );
}

void MipMapLimiter::apply( osg::Texture3D* tex )
{
    //
    // Compute the number of levels in the mipmap pyramid we wish to discard.
    //

    int levelsToRemove( 0 );

    int w = tex->getTextureWidth();
    int h = tex->getTextureHeight();
    int d = tex->getTextureDepth();
    if( _limitMode == MAX_DIMENSION )
    {
        int origW( w );
        int origH( h );
        int origD( d );
        w = clampPowerOf2( osg::minimum< int >( w, (int)_limitValue ) );
        h = clampPowerOf2( osg::minimum< int >( h, (int)_limitValue ) );
        d = clampPowerOf2( osg::minimum< int >( d, (int)_limitValue ) );
        int wLevels, hLevels, dLevels;
        for( wLevels=0; origW > w; wLevels++, origW >>= 1 ) {}
        for( hLevels=0; origH > h; hLevels++, origH >>= 1 ) {}
        for( dLevels=0; origD > d; dLevels++, origD >>= 1 ) {}
        levelsToRemove = osg::maximum< int >( wLevels, hLevels );
        levelsToRemove = osg::maximum< int >( levelsToRemove, dLevels );
    }
    else if( _limitMode == REMOVE_LEVELS )
    {
        levelsToRemove = _limitValue;
        w >>= _limitValue;
        h >>= _limitValue;
        d >>= _limitValue;
        w = osg::maximum< int >( w, 1 );
        h = osg::maximum< int >( h, 1 );
        d = osg::maximum< int >( d, 1 );
    }
    if( ( w == tex->getTextureWidth() ) &&
        ( h == tex->getTextureHeight() ) &&
        ( d == tex->getTextureDepth() ) )
    {
        // Nothing to do.
        return;
    }
    totalTexturesExceedingMaxDimension++;


    //
    // Get the mipmap level from the hardware.
    //

    osg::Image* newImage = readImageFromCurrentTexture( _contextID, levelsToRemove );
    if( newImage == NULL )
    {
        // readImageFromCurrentTexture() displays a message; set notify DEBUG_FP to see it.
        return;
    }

    tex->setImage( newImage );
    tex->setUseHardwareMipMapGeneration( true );
}

void MipMapLimiter::apply( osg::TextureCubeMap* tex )
{
    totalUnsupported++;
}

void MipMapLimiter::apply( osg::TextureRectangle* tex )
{
    totalUnsupported++;
}


unsigned int MipMapLimiter::clampPowerOf2( const unsigned int in )
{
    // If 'in' == 0, that's invalid. And if 'in' is 1 or 2, that's
    // already a power of 2, so check for this trivial case and return.
    if( in <= 2 ) return( in );
    // Beyond this point, 'in' is >= 3.

    if( ( in & (in-1) ) == 0 )
        // It's already a power of 2, return it.
        return( in );

    // Strategy: Find the next highest power of 2, then
    // shift right 1 to get the next lowest power of 2.
    unsigned int result = in;
    result |= result >> 1;
    result |= result >> 2;
    result |= result >> 4;
    result |= result >> 8;
    const int bytes = sizeof( unsigned int );
    if( bytes > 2 ) // I hope so, probably 4 bytes for a 32-bit build
        result |= result >> 16;
    // For now, don't support 64-bit builds, we'll never see a texture dimension this big anyway.
    //if( bytes > 4 ) // 64-bit build
    //    result |= result >> 32;
    result++;
    return( result >> 1 );
}



// osgwTools
}
