// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#ifndef __DEPTH_PEEL_GROUP_H__
#define __DEPTH_PEEL_GROUP_H__ 1


#include <osg/Object>
#include <osg/Group>
#include <osg/Camera>
#include <osg/NodeVisitor>
#include <osg/FrameBufferObject>
#include <osgUtil/CullVisitor>

#include "DepthPeelRenderStage.h"

#include <string>




class DepthPeelGroup : public osg::Group
{
public:
    DepthPeelGroup();
    DepthPeelGroup( const DepthPeelGroup& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY );

    META_Node(myLib,DepthPeelGroup);

    // Set the number of passes to perform. Default is 6. Set to 0 to
    // render as many passes as required. Number of passes is always
    // clamped to GL_MAX_TEXTURE_IMAGE_UNITS. The first pass is always
    // an opaque pass.
    void setNumPasses( unsigned int numPasses );
    unsigned int getNumPasses() const;

    // If the number of passes is set to 0, this function controls the
    // minimum number of passes that will be performed. Additional passes
    // are performed with occlusion query until the number of rendered
    // pixels reaches zero. Default min passes is 5.
    void setMinPasses( unsigned int minPasses );
    unsigned int getMinPasses() const;

    // When enabled (default), this group inserts a custom RenderStage
    // during the cull traversal, which renders multiple passes to effect
    // depth peeling. When disabled, the custom RenderStage is not inserted,
    // and this group behaves as a normal OSG Group.
    void setEnable( bool enable ) { _enable = enable; }
    bool getEnable() const { return( _enable ); }

    // Specify the distination for the final image. Pass NULL (the
    // default) to render to the default FBO (from the window system).
    void setDestination( osg::FrameBufferObject* dest );
    osg::FrameBufferObject* getDestination() const;

    // Specify the texture unit to be used for the depth texture. Default
    // is zero, but you might need to change this if your subgraph uses
    // unit 0 for another texture.
    void setTextureUnit( unsigned int unit );
    unsigned int getTextureUnit() const;


    // 
    // Internal use; do not call from app.
    // Made public for easy access by DepthPeelRenderStage.
    //

    virtual void traverse( osg::NodeVisitor& nv );

    const osg::Geometry& getFinalQuad() const { return( *_finalQuad ); }

    void setRenderingCache( osg::Object* rc ) { _renderingCache = rc; }
    osg::Object* getRenderingCache() { return _renderingCache.get(); }
    const osg::Object* getRenderingCache() const { return _renderingCache.get(); }

    void resizeGLObjectBuffers( unsigned int maxSize );
    void releaseGLObjects( osg::State* state ) const;

protected:
    ~DepthPeelGroup();

    void internalInit();
    osg::ref_ptr< osg::Geometry > _finalQuad;

    unsigned int _numPasses, _minPasses;
    bool _enable;
    osg::ref_ptr< osg::FrameBufferObject > _destination;
    unsigned int _textureUnit;

    osg::ref_ptr< osg::Object > _renderingCache;
};

// __DEPTH_PEEL_GROUP_H__
#endif
