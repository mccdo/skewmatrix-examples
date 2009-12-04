// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osg/Object>
#include <osg/Group>
#include <osg/Camera>
#include <osg/Geometry>
#include <osg/Program>
#include <osg/NodeVisitor>
#include <osgUtil/RenderBin>
#include <osgUtil/RenderStage>
#include <osgUtil/CullVisitor>
#include <osgDB/FileUtils>

#include "DepthPeelRenderStage.h"
#include "DepthPeelGroup.h"

#include <string>



// Lifted directly from CullVisitor
class RenderStageCache : public osg::Object
{
public:
    RenderStageCache() {}
    RenderStageCache(const RenderStageCache&, const osg::CopyOp&) {}

    META_Object(myLib,RenderStageCache);

    void setRenderStage( osgUtil::CullVisitor* cv, DepthPeelRenderStage* rs)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        _renderStageMap[cv] = rs;
    }        

    DepthPeelRenderStage* getRenderStage( osgUtil::CullVisitor* cv )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return _renderStageMap[cv].get();
    }

    typedef std::map< osgUtil::CullVisitor*, osg::ref_ptr< DepthPeelRenderStage > > RenderStageMap;

    OpenThreads::Mutex  _mutex;
    RenderStageMap      _renderStageMap;
};



DepthPeelGroup::DepthPeelGroup()
  : _numPasses( 6 ),
    _minPasses( 5 ),
    _enable( true ),
    _destination( NULL ),
    _textureUnit( 0 )
{
    internalInit();
}

DepthPeelGroup::DepthPeelGroup( const DepthPeelGroup& rhs, const osg::CopyOp& copyOp )
  : _numPasses( rhs._numPasses ),
    _minPasses( rhs._minPasses ),
    _enable( rhs._enable ),
    _destination( rhs._destination ),
    _textureUnit( rhs._textureUnit )
{
    internalInit();
}

DepthPeelGroup::~DepthPeelGroup()
{
}

void
DepthPeelGroup::internalInit()
{
    _finalQuad = osg::createTexturedQuadGeometry(
        osg::Vec3( -1,-1,0 ), osg::Vec3( 2,0,0 ), osg::Vec3( 0,2,0 ) );
    _finalQuad->setUseDisplayList( false );
    _finalQuad->setUseVertexBufferObjects( true );

    osg::StateSet* ss( _finalQuad->getOrCreateStateSet() );

    osg::Shader* vertShader = new osg::Shader( osg::Shader::VERTEX );
    vertShader->loadShaderSourceFromFile( osgDB::findDataFile( "DepthPeelFinal.vs" ) );
    osg::Shader* fragShader = new osg::Shader( osg::Shader::FRAGMENT );
    fragShader->loadShaderSourceFromFile( osgDB::findDataFile( "DepthPeelFinal.fs" ) );

    osg::Program* program = new osg::Program();
    program->addShader(vertShader);
    program->addShader(fragShader);
    ss->setAttribute( program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    ss->addUniform( new osg::Uniform( "depthPeelLayerMap", (int)( _textureUnit ) ) );
}

void
DepthPeelGroup::traverse( osg::NodeVisitor& nv )
{
    if( ( nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR ) ||
        ( !_enable ) )
    {
        // Not the cull visitor, or depth peeling is not enabled.
        // Traverse as a Group and return.
        osg::Group::traverse( nv );
        return;
    }

    // Dealing with cull traversal from this point onwards.
    osgUtil::CullVisitor* cv = static_cast< osgUtil::CullVisitor* >( &nv );

    //
    // Basic idea of what follows was derived from CullVisitor::apply( Camera& ).
    //

    osgUtil::RenderStage* previousStage = cv->getCurrentRenderBin()->getStage();
    osg::Camera* camera = previousStage->getCamera();

    osg::ref_ptr< RenderStageCache > rsCache = dynamic_cast< RenderStageCache* >( getRenderingCache() );
    if( !rsCache )
    {
        rsCache = new RenderStageCache;
        setRenderingCache( rsCache.get() );
    }

    osg::ref_ptr< DepthPeelRenderStage > dprs = rsCache->getRenderStage( cv );
    if( !dprs )
    {
        dprs = new DepthPeelRenderStage( *previousStage );
        dprs->setDepthPeelGroup( this );
        rsCache->setRenderStage( cv, dprs.get() );
    }
    else
    {
        // Reusing custom RenderStage. Reset it to clear previous cull's contents.
        dprs->reset();
    }

    // Set DPRS viewport
    osg::Viewport* viewport = ( camera->getViewport() != NULL ) ?
        camera->getViewport() : previousStage->getViewport();
    dprs->setViewport( viewport );

    // TBD allow the app the specify this.
    dprs->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );



    {
        // Save RenderBin
        osgUtil::RenderBin* previousRenderBin = cv->getCurrentRenderBin();
        cv->setCurrentRenderBin( dprs.get() );

        // Traverse
        osg::Group::traverse( nv );

        // Restore RenderBin
        cv->setCurrentRenderBin( previousRenderBin );
    }


    // Hook our RenderStage into the render graph.
    // TBD Might need to support RenderOrder a la Camera node. For now, pre-render.
    cv->getCurrentRenderBin()->getStage()->addPreRenderStage( dprs.get(), camera->getRenderOrderNum() );
}


void
DepthPeelGroup::setNumPasses( unsigned int numPasses )
{
    _numPasses = numPasses;
}
unsigned int
DepthPeelGroup::getNumPasses() const
{
    return _numPasses;
}

void
DepthPeelGroup::setMinPasses( unsigned int minPasses )
{
    _minPasses = minPasses;
}
unsigned int
DepthPeelGroup::getMinPasses() const
{
    return _minPasses;
}

void
DepthPeelGroup::setDestination( osg::FrameBufferObject* dest )
{
    _destination = dest;
}

osg::FrameBufferObject*
DepthPeelGroup::getDestination() const
{
    return( _destination.get() );
}

void
DepthPeelGroup::setTextureUnit( unsigned int unit )
{
    if( _textureUnit != unit )
    {
        _textureUnit = unit;
        osg::StateSet* ss( _finalQuad->getOrCreateStateSet() );
        ss->getUniform( "depthPeelLayerMap" )->set( (int)( unit ) );
    }
}

unsigned int
DepthPeelGroup::getTextureUnit() const
{
    return( _textureUnit );
}


void
DepthPeelGroup::resizeGLObjectBuffers( unsigned int maxSize )
{
    if( _renderingCache.valid() )
        const_cast< DepthPeelGroup* >( this )->_renderingCache->resizeGLObjectBuffers( maxSize );

    osg::Group::resizeGLObjectBuffers(maxSize);
}

void
DepthPeelGroup::releaseGLObjects( osg::State* state ) const
{
    if( _renderingCache.valid() )
        const_cast< DepthPeelGroup* >( this )->_renderingCache->releaseGLObjects( state );

    osg::Group::releaseGLObjects(state);
}
