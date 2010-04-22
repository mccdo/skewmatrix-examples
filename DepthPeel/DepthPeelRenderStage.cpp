// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osgwTools/Version.h>
#include <osg/Node>
#include <osg/Group>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgUtil/RenderStage>
#include <osgUtil/CullVisitor>
#include <osg/GLExtensions>
#include <osg/FrameBufferObject>

#include "DepthPeelRenderStage.h"
#include "DepthPeelGroup.h"

#include <string>




DepthPeelRenderStage::PerLayerInfo::PerLayerInfo()
  : _init( false ),
    _fbo( -1 ),
    _colorTex( -1 ),
    _depthTex( -1 )
{
}

void
DepthPeelRenderStage::PerLayerInfo::init( unsigned int pass, const osg::State& state, const osg::Viewport* vp, const PerContextInfo& ctxInfo )
{
    osg::notify( osg::DEBUG_FP ) << "DPRS: PerLayerInfo::init()" << std::endl;

    const unsigned int contextID( state.getContextID() );
    osg::FBOExtensions* fboExt( osg::FBOExtensions::instance( contextID, true ) );

#if( OSGWORKS_OSG_VERSION > 20907 )
    fboExt->glGenFramebuffers( 1, &_fbo );
    fboExt->glBindFramebuffer( GL_DRAW_FRAMEBUFFER_EXT, _fbo );
#else
    fboExt->glGenFramebuffersEXT( 1, &_fbo );
    fboExt->glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, _fbo );
#endif

    GLsizei winW( osg::maximum< int >( vp->x(), 0 ) + vp->width() );
    GLsizei winH( osg::maximum< int >( vp->y(), 0 ) + vp->height() );

    // TBD Give the app a way to specify the FBO attachment combination,
    // e.g., color, depth, stencil, MRT, etc.

    // Color -- Create and attach the texture for color.
    glGenTextures( 1, &_colorTex );
    glBindTexture( GL_TEXTURE_2D, _colorTex );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, winW, winH,
        0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glBindTexture( GL_TEXTURE_2D, 0 );

    // Obtain depth texture ID from PerContextInfo.
    // pass N gets
    //   N odd: PerContextInfo depthTex[ 1 ]
    //   N even: PerContextInfo depthTex[ 0 ]
    _depthTex = ctxInfo._depthTex[ pass & 0x1 ];
    osg::notify( osg::DEBUG_FP ) << "DPRS: Pass " << pass << ", idx " << (pass & 0x1) << ", depthTex " << _depthTex << std::endl;

#if( OSGWORKS_OSG_VERSION > 20907 )
    fboExt->glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER_EXT,
        GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _colorTex, 0 );
    fboExt->glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER_EXT,
        GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, _depthTex, 0 );
#else
    fboExt->glFramebufferTexture2DEXT( GL_DRAW_FRAMEBUFFER_EXT,
        GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _colorTex, 0 );
    fboExt->glFramebufferTexture2DEXT( GL_DRAW_FRAMEBUFFER_EXT,
        GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, _depthTex, 0 );
#endif

    _init = true;
}


DepthPeelRenderStage::PerContextInfo::PerContextInfo()
  : _init( false ),
    _glActiveTexture( 0 ),
    _maximumPasses( 8 ),
    _queryID( 0 )
{
    _depthTex[ 0 ] = _depthTex[ 1 ] = _depthTex[ 2 ] = 0;
}

void
DepthPeelRenderStage::PerContextInfo::init( GLuint unit, const osg::Viewport* vp )
{
    osg::notify( osg::DEBUG_FP ) << "DPRS: PerContextInfo::init()" << std::endl;

    osg::setGLExtensionFuncPtr( _glActiveTexture, "glActiveTexture" );

    osg::setGLExtensionFuncPtr( _glGenQueries, "glGenQueries", "glGenQueriesARB");
    osg::setGLExtensionFuncPtr( _glDeleteQueries, "glDeleteQueries", "glDeleteQueriesARB");
    osg::setGLExtensionFuncPtr( _glBeginQuery, "glBeginQuery", "glBeginQueryARB");
    osg::setGLExtensionFuncPtr( _glEndQuery, "glEndQuery", "glEndQueryARB");
    osg::setGLExtensionFuncPtr( _glGetQueryObjectiv, "glGetQueryObjectiv","glGetQueryObjectivARB");

    _glGenQueries( 1, &_queryID );

    GLint maxUnits;
    glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &maxUnits );
    _maximumPasses = (unsigned int)( maxUnits );

    GLsizei winW( osg::maximum< int >( vp->x(), 0 ) + vp->width() );
    GLsizei winH( osg::maximum< int >( vp->y(), 0 ) + vp->height() );

    // Create two depth buffers; First two are ping-pong buffers for each pass,
    // third is cleared to the near plane for the initial pass.
    glGenTextures( 3, _depthTex );
    _glActiveTexture( GL_TEXTURE0 + unit );
    int idx;
    for( idx=0; idx<3; idx++ )
    {
        glBindTexture( GL_TEXTURE_2D, _depthTex[ idx ] );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_ALPHA );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_GREATER );

        if( idx==2 )
        {
            GLuint* data = new GLuint[ winW * winH ];
            memset( data, 0, winW * winH * sizeof(GLuint) );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, winW, winH,
                0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, data );
            delete[] data;
        }
        else
        {
            glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, winW, winH,
                0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );
        }
    }
    glBindTexture( GL_TEXTURE_2D, 0 );

    _init = true;
}



DepthPeelRenderStage::DepthPeelRenderStage()
  : osgUtil::RenderStage(),
    _dpg( NULL )
{
    internalInit();
}

DepthPeelRenderStage::DepthPeelRenderStage( const osgUtil::RenderStage& rhs, const osg::CopyOp& copyop )
  : osgUtil::RenderStage( rhs )
{
    internalInit();
}
DepthPeelRenderStage::DepthPeelRenderStage( const DepthPeelRenderStage& rhs, const osg::CopyOp& copyop )
  : osgUtil::RenderStage( rhs ),
    _dpg( rhs._dpg )
{
    internalInit();
}

DepthPeelRenderStage::~DepthPeelRenderStage()
{
}


void
DepthPeelRenderStage::internalInit()
{
}


void
DepthPeelRenderStage::draw( osg::RenderInfo& renderInfo, osgUtil::RenderLeaf*& previous )
{
    if( _stageDrawnThisFrame )
        return;
    _stageDrawnThisFrame = true;

    osg::notify( osg::DEBUG_FP ) << "DepthPeelRenderStage::draw" << std::endl;


    //
    // Initialization

    osg::State& state( *renderInfo.getState() );
    const unsigned int contextID( state.getContextID() );
    osg::FBOExtensions* fboExt( osg::FBOExtensions::instance( contextID, true ) );
    if( fboExt == NULL )
    {
        osg::notify( osg::WARN ) << "DPRS: FBOExtensions == NULL." << std::endl;
        return;
    }
    osg::GL2Extensions* gl2Ext( osg::GL2Extensions::Get( contextID, true ) );
    if( gl2Ext == NULL )
    {
        osg::notify( osg::WARN ) << "DPRS: GL2Extensions == NULL." << std::endl;
        return;
    }

    PerContextInfo& ctxInfo( _contextInfo[ contextID ] );
    if( !ctxInfo._init )
        ctxInfo.init( _dpg->getTextureUnit(), getViewport() );

    // requestedPasses: The number of passes requested by the app
    // using DepthPeelGroup::setNumPasses( n );
    // If requestedPasses > 0:
    //   Then we perform exactly requestedPasses number of passes.
    //   minPasses = maxPasses = requestedPasses.
    // else if requestedPasses == 0:
    //   Then we do at lease minPasses number of passes (and this is done without occ query)
    //   and we might do up to maxPasses (using occ query to determine when to stop)
    unsigned int requestedPasses( _dpg->getNumPasses() );
    unsigned int minPasses, maxPasses;
    if( requestedPasses == 0 )
    {
        minPasses = _dpg->getMinPasses();
        maxPasses = ctxInfo._maximumPasses;
        if( minPasses > maxPasses )
            minPasses = maxPasses;
    }
    else if( requestedPasses > ctxInfo._maximumPasses )
    {
        minPasses = maxPasses = ctxInfo._maximumPasses;
    }
    else
    {
        minPasses = maxPasses = requestedPasses;
    }

    // Initialize per-pass/layer info (FBOs, textures, etc).
    if( ctxInfo._layersList.size() < maxPasses )
    {
        ctxInfo._layersList.resize( maxPasses );

        ctxInfo._glActiveTexture( GL_TEXTURE0 + _dpg->getTextureUnit() );
        unsigned int idx;
        for( idx=0; idx<maxPasses; idx++ )
        {
            if( !( ctxInfo._layersList[ idx ]._init ) )
                ctxInfo._layersList[ idx ].init( idx, state, getViewport(), ctxInfo );
        }

        checkFBOStatus( fboExt );
        errorCheck( "in DPRS initialization" );
    }

    // End initialization
    //


    //
    // Layer creation

    drawStart( renderInfo, previous );

    // Count the number of actual passes. If numPasses > 0, then
    // passCount will equal numPasses. But if numPasses == 0, then
    // passCount will equal the actual number of passes rendered.
    unsigned int passCount( 0 );

    while( passCount < minPasses )
    {
        peelBegin( passCount, ctxInfo, state, fboExt, gl2Ext );

        RenderBin::drawImplementation( renderInfo, previous );

        peelEnd( passCount );
        passCount++;
    }

    // If app has called setNumPasses( 0 ), do additional passes
    // until occ query indicates very few pixels were rendered.
    while( passCount < maxPasses )
    {
        peelBegin( passCount, ctxInfo, state, fboExt, gl2Ext );

        ctxInfo._glBeginQuery( GL_SAMPLES_PASSED_ARB, ctxInfo._queryID );
        RenderBin::drawImplementation( renderInfo, previous );
        ctxInfo._glEndQuery( GL_SAMPLES_PASSED_ARB );

        GLint numPixels;
        ctxInfo._glGetQueryObjectiv( ctxInfo._queryID, GL_QUERY_RESULT, &numPixels );
        osg::notify( osg::DEBUG_FP ) << "  numPixels " << numPixels << std::endl;
        if( numPixels < 25 ) // TBD hardcoded min resolvable value
            break;

        peelEnd( passCount );
        passCount++;
    }

    drawEnd( renderInfo, previous );

    osg::notify( osg::DEBUG_FP ) << "ctx " << contextID <<
        ", passCount " << passCount <<
        ", minPasses " << minPasses <<
        ", maxPasses " << maxPasses <<
        ", requestedPasses " << requestedPasses << std::endl;

    // End layer creation
    //


    //
    // Composite layers

    // Final render pass, accumulating all depth peel layers
    // and displaying on the specified FBO.
    osg::FrameBufferObject* destination = _dpg->getDestination();
    if( destination == NULL )
    {
#if( OSGWORKS_OSG_VERSION > 20907 )
        fboExt->glBindFramebuffer( GL_DRAW_FRAMEBUFFER_EXT, 0 );
#else
        fboExt->glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
#endif
        glDrawBuffer( GL_BACK ); // HACK TBD should use _drawBuffer, and _drawBuffer needs to be set properly.
    }
    else
        _fbo->apply( state, osg::FrameBufferObject::DRAW_FRAMEBUFFER );

    state.applyAttribute( getViewport() );

    state.applyAttribute( _dpg->getFinalQuad().getStateSet()->getAttribute( osg::StateAttribute::PROGRAM ) );
    _dpg->getFinalQuad().getStateSet()->getUniform( "depthPeelLayerMap" )->apply( gl2Ext, 0 );

    {
        // Save depth and blend state
        const bool depthEnabled( state.getLastAppliedMode( GL_DEPTH_TEST ) );
        state.applyMode( GL_DEPTH_TEST, false );
        glDepthMask( GL_FALSE );
        const bool blendEnabled( state.getLastAppliedMode( GL_BLEND ) );
        const osg::StateAttribute* blendAttr( state.getLastAppliedAttribute( osg::StateAttribute::BLENDFUNC ) );
        state.applyMode( GL_BLEND, true );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        ctxInfo._glActiveTexture( GL_TEXTURE0 + _dpg->getTextureUnit() );
        int idx;
        for( idx=passCount-1; idx>=0; idx-- )
        {
            glBindTexture( GL_TEXTURE_2D, ctxInfo._layersList[ idx ]._colorTex );
            _dpg->getFinalQuad().draw( renderInfo );
        }

        // Restore depth and blend state
        state.applyMode( GL_DEPTH_TEST, depthEnabled );
        glDepthMask( GL_TRUE );
        state.applyMode( GL_BLEND, blendEnabled );
        if( blendAttr != NULL )
            state.applyAttribute( blendAttr );
    }

    // End composite layers
    //


    if( state.getCheckForGLErrors() != osg::State::NEVER_CHECK_GL_ERRORS )
    {
        errorCheck( "at DPRS draw end" );
        checkFBOStatus( fboExt );
    }
}

void
DepthPeelRenderStage::drawStart( osg::RenderInfo& renderInfo, osgUtil::RenderLeaf*& previous )
{
    osg::State& state = *renderInfo.getState();

    if (!_viewport)
    {
        osg::notify( osg::FATAL ) << "Error: cannot draw stage due to undefined viewport."<< std::endl;
        return;
    }

    // set up the back buffer.
    state.applyAttribute(_viewport.get());

    glScissor( static_cast<int>(_viewport->x()),
               static_cast<int>(_viewport->y()),
               static_cast<int>(_viewport->width()),
               static_cast<int>(_viewport->height()) );
    //cout << "    clearing "<<this<< "  "<<_viewport->x()<<","<< _viewport->y()<<","<< _viewport->width()<<","<< _viewport->height()<<std::endl;
    state.applyMode( GL_SCISSOR_TEST, true );

    // glEnable( GL_DEPTH_TEST );

    // set which color planes to operate on.
    if (_colorMask.valid()) _colorMask->apply(state);
    else glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

    if (_clearMask & GL_COLOR_BUFFER_BIT)
    {
        glClearColor( _clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]);
    }

    if (_clearMask & GL_DEPTH_BUFFER_BIT)
    {
        glClearDepth( _clearDepth);
        glDepthMask ( GL_TRUE );
        state.haveAppliedAttribute( osg::StateAttribute::DEPTH );
    }

    if (_clearMask & GL_STENCIL_BUFFER_BIT)
    {
        glClearStencil( _clearStencil);
        glStencilMask ( ~0u );
        state.haveAppliedAttribute( osg::StateAttribute::STENCIL );
    }

    if (_clearMask & GL_ACCUM_BUFFER_BIT)
    {
        glClearAccum( _clearAccum[0], _clearAccum[1], _clearAccum[2], _clearAccum[3]);
    }


#ifdef USE_SCISSOR_TEST
    glDisable( GL_SCISSOR_TEST );
#endif

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // apply the positional state.
    if (_inheritedPositionalStateContainer.valid())
    {
        _inheritedPositionalStateContainer->draw(state, previous, &_inheritedPositionalStateContainerMatrix);
    }

    // apply the positional state.
    if (_renderStageLighting.valid())
    {
        _renderStageLighting->draw(state, previous, 0);
    }
}
void
DepthPeelRenderStage::drawEnd( osg::RenderInfo& renderInfo, osgUtil::RenderLeaf*& previous )
{
    osg::State& state = *renderInfo.getState();
    state.apply();
}


void
DepthPeelRenderStage::peelBegin( unsigned int pass, PerContextInfo& ctxInfo, osg::State& state, osg::FBOExtensions* fboExt, osg::GL2Extensions* gl2Ext )
{
    PerLayerInfo layerInfo( ctxInfo._layersList[ pass ] );
#if( OSGWORKS_OSG_VERSION > 20907 )
    fboExt->glBindFramebuffer( GL_DRAW_FRAMEBUFFER_EXT, layerInfo._fbo );
#else
    fboExt->glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, layerInfo._fbo );
#endif
    glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );

    GLuint depthTexID;
    if( pass == 0 )
    {
        depthTexID = ctxInfo._depthTex[ 2 ];
    }
    else
    {
        PerLayerInfo prevLayerInfo( ctxInfo._layersList[ pass - 1 ] );
        depthTexID = prevLayerInfo._depthTex;
    }
    ctxInfo._glActiveTexture( GL_TEXTURE0 + _dpg->getTextureUnit() );
    state.setActiveTextureUnit( _dpg->getTextureUnit() );
    glBindTexture( GL_TEXTURE_2D, depthTexID );
    state.haveAppliedTextureAttribute( _dpg->getTextureUnit(), osg::StateAttribute::TEXTURE );

    glClear( _clearMask );

    errorCheck( "in DPRS peelBegin" );

    // TBD this doesn't seem to fix our problem.
    /*
    state.haveAppliedAttribute( osg::StateAttribute::VIEWPORT );
    state.haveAppliedAttribute( osg::StateAttribute::DEPTH );
    state.haveAppliedAttribute( osg::StateAttribute::TEXTURE );
    */
}
void
DepthPeelRenderStage::peelEnd( unsigned int pass )
{
    // TBD this doesn't seem to fix our problem.
    /*
    // Tell OSG we dirtied some state.
    state.haveAppliedAttribute( osg::StateAttribute::VIEWPORT );
    state.haveAppliedAttribute( osg::StateAttribute::DEPTH );
    state.haveAppliedAttribute( osg::StateAttribute::TEXTURE );
    state.dirtyVertexPointer();
    state.dirtyTexCoordPointer( 0 );
    state.dirtyColorPointer();
    state.dirtyNormalPointer();
    */

    errorCheck( "in DPRS peelEnd" );
}

void
DepthPeelRenderStage::errorCheck( const std::string& msg )
{
    GLenum errorNo = glGetError();
    if( errorNo == GL_NO_ERROR )
        return;

    osg::notify( osg::WARN ) << "DPRS: OpenGL error 0x" << std::hex << errorNo << std::dec << " ";
    switch( errorNo ) {
    case GL_INVALID_ENUM: osg::notify( osg::WARN ) << "GL_INVALID_ENUM"; break;
    case GL_INVALID_VALUE: osg::notify( osg::WARN ) << "GL_INVALID_VALUE"; break;
    case GL_INVALID_OPERATION: osg::notify( osg::WARN ) << "GL_INVALID_OPERATION"; break;
#if defined( OSG_GL1_AVAILABLE ) || defined( OSG_GL1_AVAILABLE )
    case GL_STACK_OVERFLOW: osg::notify( osg::WARN ) << "GL_STACK_OVERFLOW"; break;
    case GL_STACK_UNDERFLOW: osg::notify( osg::WARN ) << "GL_STACK_UNDERFLOW"; break;
#endif
    case GL_OUT_OF_MEMORY: osg::notify( osg::WARN ) << "GL_OUT_OF_MEMORY"; break;
#if defined( OSG_GL3_AVAILABLE )
    case GL_INVALID_FRAMEBUFFER_OPERATION: osg::notify( osg::WARN ) << "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
#endif
    default: osg::notify( osg::WARN ) << "Unknown"; break;
    }
    osg::notify( osg::WARN ) << " " << msg << std::endl;
}

void
DepthPeelRenderStage::checkFBOStatus( osg::FBOExtensions* fboExt )
{
#if( OSGWORKS_OSG_VERSION > 20907 )
    const GLenum fbStatus( fboExt->glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER_EXT ) );
#else
    const GLenum fbStatus( fboExt->glCheckFramebufferStatusEXT( GL_DRAW_FRAMEBUFFER_EXT ) );
#endif
    if( fbStatus == GL_FRAMEBUFFER_COMPLETE_EXT )
        return;

    osg::notify( osg::WARN ) << "DPRS: FB Status: ";
    switch( fbStatus ) {
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT: osg::notify( osg::WARN ) << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl; break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: osg::notify( osg::WARN ) << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT: osg::notify( osg::WARN ) << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: osg::notify( osg::WARN ) << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: osg::notify( osg::WARN ) << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl; break;
    default: osg::notify( osg::WARN ) << "Unknown " << std::hex << fbStatus << std::dec << std::endl; break;
    }
}
