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

#include <osg/Node>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Depth>
#include <osg/GLExtensions>
#include <osg/buffered_value>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgwTools/FBOUtils.h>
#include <osgwTools/Version.h>

#include <string>


//#define USE_MS

const int winW( 800 ), winH( 600 );


/* \cond */
class MSMRTCallback : public osg::Camera::DrawCallback
{
public:
    MSMRTCallback( osg::Camera* cam )
      : _cam( cam )
    {
    }

    virtual void operator()( osg::RenderInfo& renderInfo ) const
    {
        osg::State& state = *renderInfo.getState();
        const unsigned int ctx = state.getContextID();
        osg::FBOExtensions* fboExt = osg::FBOExtensions::instance( ctx, true );

        PerContextInfo& ctxInfo( _contextInfo[ ctx ] );
        if( ctxInfo.__glGetFramebufferAttachmentParameteriv == NULL )
        {
            // Initialize function pointer for FBO query.
            osg::setGLExtensionFuncPtr( ctxInfo.__glGetFramebufferAttachmentParameteriv, "glGetFramebufferAttachmentParameteriv" );
            if( ctxInfo.__glGetFramebufferAttachmentParameteriv == NULL )
                osg::setGLExtensionFuncPtr( ctxInfo.__glGetFramebufferAttachmentParameteriv, "glGetFramebufferAttachmentParameterivEXT" );
            if( ctxInfo.__glGetFramebufferAttachmentParameteriv == NULL )
            {
                osg::notify( osg::ALWAYS ) << "Can't get function pointer glGetFramebufferAttachmentParameteriv" << std::endl;
                return;
            }
        }

        const GLint width = _cam->getViewport()->width();
        const GLint height = _cam->getViewport()->height();

#if 0
        // Make sure something is actually bound.
        GLint drawFBO( -1 );
        glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &drawFBO );
#endif

        // BlitFramebuffer blits to all attached color buffers in the
        // draw FBO. We only want to blit to attachment1, so aave
        // attachment0 and then unbind it.
        GLint destColorTex0( -1 );
        ctxInfo.__glGetFramebufferAttachmentParameteriv(
            GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &destColorTex0 );
        osgwTools::glFramebufferTexture2D( fboExt,
            GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, 0, 0 );

        // Verification
        //osg::notify( osg::ALWAYS ) << "Dest " << std::hex << destColorTex0 << std::endl;

        // Set draw and read buffers to attachment1 to read from correct
        // buffer and avoid INVALID_FRAMEBUFFER_OPERATION error.
        glDrawBuffer( GL_COLOR_ATTACHMENT1 );
        glReadBuffer( GL_COLOR_ATTACHMENT1 );

        // Blit, from (multisampled read FBO) attachment1 to
        // (non-multisampled draw FBO) attachment1.
        osgwTools::glBlitFramebuffer( fboExt, 0, 0, width, height, 0, 0, width, height,
            GL_COLOR_BUFFER_BIT, GL_NEAREST );

        // Restore draw and read buffers
        glDrawBuffer( GL_COLOR_ATTACHMENT0 );
        glReadBuffer( GL_COLOR_ATTACHMENT0 );

        // Restore the draw FBO's attachment0.
        osgwTools::glFramebufferTexture2D( fboExt,
            GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, destColorTex0, 0 );

        // We disabled FBO unbinding in the RenderStage,
        // so do it ourself here.
        /* TBD update to use FBOUtils.h */
        osgwTools::glBindFramebuffer( fboExt, GL_FRAMEBUFFER, 0 );
    }

protected:
    osg::ref_ptr< osg::Camera > _cam;

    typedef void APIENTRY TglGetFramebufferAttachmentParameteriv( GLenum, GLenum, GLenum, GLint* );

    // Each different context could potentially have a different address for
    // the FBO query function. For this reason, keep it in buffered_value
    // and init / index the function pointer on a per-context basis.
    // Of course, this wouldn't be necessary if OSG already has this function
    // pointer in FBOExtensions. Or if OSG used something like GLEW.
    struct PerContextInfo
    {
        PerContextInfo()
        {
            __glGetFramebufferAttachmentParameteriv = NULL;
        }

        TglGetFramebufferAttachmentParameteriv* __glGetFramebufferAttachmentParameteriv;
    };
    mutable osg::buffered_object< PerContextInfo > _contextInfo;
};


// RenderStage unbinds FBOs before executing post-draw callbacks.
// The only way I know of to access the RenderStage (to disable this
// unbinding) is with a cull callback.
class KeepFBOsBoundCallback : public osg::NodeCallback
{
public:
    KeepFBOsBoundCallback() {}

    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        if( nv->getVisitorType() != osg::NodeVisitor::CULL_VISITOR )
        {
            traverse( node, nv );
            return;
        }

        // Get the current RenderStage and prevent it from unbinding the FBOs
        // just before our post-draw MSMRTCallback is executed. We need them
        // bound in our callback so we can execute another glBlitFramebuffer.
        // After the blit, MSMRTCallback unbinds the FBOs.
        osgUtil::CullVisitor* cv = static_cast< osgUtil::CullVisitor* >( nv );
        // Don't use getRenderStage(). It returns the _root_ RenderStage.
        //osgUtil::RenderStage* rs = cv->getRenderStage();
        osgUtil::RenderStage* rs = cv->getCurrentRenderBin()->getStage();
        rs->setDisableFboAfterRender( false );

        traverse( node, nv );
    }
};
/* \endcond */


osg::Node* fsQuad( osg::StateSet* stateSet, const std::string& name )
{
    osg::ref_ptr< osg::Geode > geode( new osg::Geode );
    geode->setName( name );
    geode->setCullingActive( false );

    geode->addDrawable( osg::createTexturedQuadGeometry(
        osg::Vec3( -1,-1,0 ), osg::Vec3( 2,0,0 ), osg::Vec3( 0,2,0 ) ) );
    geode->setStateSet( stateSet );

    return( geode.release() );
}



// State set for writing two color values. Used when rendering
// main scene graph.
void mrtStateSet( osg::StateSet* ss )
{
    ss->setName( "mrtStateSet" );

    ss->addUniform( new osg::Uniform( "tex", 0 ) );

    std::string fragSource = 
        "uniform sampler2D tex; \n"
        "void main() \n"
        "{ \n"
            "gl_FragData[0] = texture2D( tex, gl_TexCoord[0].st ); \n"
            "gl_FragData[1] = vec4( 0.0, 0.0, 1.0, 0.0 ); \n"
        "} \n";
    osg::Shader* fragShader = new osg::Shader();
    fragShader->setName( "MRTShader Fragment" );
    fragShader->setType( osg::Shader::FRAGMENT );
    fragShader->setShaderSource( fragSource );

    osg::Program* program = new osg::Program();
    program->addShader( fragShader );
    ss->setAttribute( program, osg::StateAttribute::ON );
}

osg::StateSet* clearStateSet()
{
    osg::ref_ptr< osg::StateSet > stateSet = new osg::StateSet();
    stateSet->setName( "clearStateSet" );

    std::string vertSource = 
        "void main() \n"
        "{ \n"
            "gl_Position = gl_Vertex; \n"
        "} \n";
    osg::Shader* vertShader = new osg::Shader( osg::Shader::VERTEX, vertSource );
    vertShader->setName( "ClearShader Vertex" );

    std::string fragSource = 
        "void main() \n"
        "{ \n"
            // Clear the main color buffer
            "gl_FragData[0] = vec4( 0.25, 0., 0., 1. ); \n"
            // Clear the pseudo glow color buffer
            "gl_FragData[1] = vec4( 0., 0., 0., 1. ); \n"
            "gl_FragDepth = 1.; \n"
        "} \n";
    osg::Shader* fragShader = new osg::Shader( osg::Shader::FRAGMENT, fragSource );
    fragShader->setName( "ClearShader Fragment" );

    osg::Program* program = new osg::Program();
    program->setName( "ClearProgram" );
    program->addShader( vertShader );
    program->addShader( fragShader );
    stateSet->setAttributeAndModes( program );

    osg::Depth* depth = new osg::Depth( osg::Depth::ALWAYS );
    stateSet->setAttributeAndModes( depth );

    return( stateSet.release() );
}

// State set for combining two textures. Used
// when rendering fullscreen tri pairs.
osg::StateSet* combineStateSet( osg::Texture2D* tex0, osg::Texture2D* tex1 )
{
    osg::ref_ptr< osg::StateSet > stateSet = new osg::StateSet();
    stateSet->setName( "combineStateSet" );

    stateSet->setTextureAttributeAndModes( 0, tex0, osg::StateAttribute::ON );
    stateSet->setTextureAttributeAndModes( 1, tex1, osg::StateAttribute::ON );

    stateSet->addUniform( new osg::Uniform( "tex0", 0 ) );
    stateSet->addUniform( new osg::Uniform( "tex1", 1 ) );

    std::string vertSource = 
        "void main() \n"
        "{ \n"
            "gl_Position = gl_Vertex; \n"
            "gl_TexCoord[ 0 ] = gl_MultiTexCoord0; \n"
        "} \n";
    osg::Shader* vertShader = new osg::Shader( osg::Shader::VERTEX, vertSource );
    vertShader->setName( "CombineShader Vertex" );

    std::string fragSource = 
        "uniform sampler2D tex0; \n"
        "uniform sampler2D tex1; \n"
        "void main() \n"
        "{ \n"
            "gl_FragData[0] = texture2D( tex0, gl_TexCoord[0].st ) \n"
                " + texture2D( tex1, gl_TexCoord[0].st ); \n"
        "} \n";
    osg::Shader* fragShader = new osg::Shader( osg::Shader::FRAGMENT, fragSource );
    fragShader->setName( "CombineShader Fragment" );

    osg::Program* program = new osg::Program();
    program->setName( "CombineProgram" );
    program->addShader( vertShader );
    program->addShader( fragShader );
    stateSet->setAttributeAndModes( program );

    stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

    return( stateSet.release() );
}


void configureCameras( osg::Camera* rootCamera, osg::Camera* preRender, osg::Camera* postRender )
{
    rootCamera->setName( "rootCamera" );
    preRender->setName( "preRender" );
    postRender->setName( "postRender" );

    // MRT: Attach two color buffers to the root & pre cameras, one for
    // the standard color image, and another for the glow color.
    osg::Texture2D* tex0 = new osg::Texture2D;
    tex0->setTextureWidth( winW );
    tex0->setTextureHeight( winH );
    tex0->setInternalFormat( GL_RGBA );
    tex0->setBorderWidth( 0 );
    tex0->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex0->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
    // Full color: attachment 0
#ifdef USE_MS
    rootCamera->attach( osg::Camera::COLOR_BUFFER0, tex0, 0, 0, false, 8, 8 );
    preRender->attach( osg::Camera::COLOR_BUFFER0, tex0, 0, 0, false, 8, 8 );
#else
    rootCamera->attach( osg::Camera::COLOR_BUFFER0, tex0 );
    preRender->attach( osg::Camera::COLOR_BUFFER0, tex0 );
#endif

    osg::Texture2D* tex1 = new osg::Texture2D;
    tex1->setTextureWidth( winW );
    tex1->setTextureHeight( winH );
    tex1->setInternalFormat( GL_RGBA );
    tex1->setBorderWidth( 0 );
    tex1->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex1->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
    // Glow color: attachment 1
#ifdef USE_MS
    rootCamera->attach( osg::Camera::COLOR_BUFFER1, tex1, 0, 0, false, 8, 8 );
    preRender->attach( osg::Camera::COLOR_BUFFER1, tex1, 0, 0, false, 8, 8 );
#else
    rootCamera->attach( osg::Camera::COLOR_BUFFER1, tex1 );
    preRender->attach( osg::Camera::COLOR_BUFFER1, tex1 );
#endif

    // Pre-tender Camera and root Camera must share the depth buffer, but there's no
    // way to set an OpenGL RenderBuffer in OSG. So, make a texture that we will
    // never reference.
    osg::Texture2D* depthTex = new osg::Texture2D;
    depthTex->setTextureSize( winW, winH );
    depthTex->setInternalFormat( GL_DEPTH_COMPONENT );
    depthTex->setBorderWidth( 0 );
    depthTex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    depthTex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
#ifdef USE_MS
    rootCamera->attach( osg::Camera::DEPTH_BUFFER, depthTex, 0, 0, false, 8, 8 );
    preRender->attach( osg::Camera::DEPTH_BUFFER, depthTex, 0, 0, false, 8, 8 );
#else
    rootCamera->attach( osg::Camera::DEPTH_BUFFER, depthTex );
    preRender->attach( osg::Camera::DEPTH_BUFFER, depthTex );
#endif

    rootCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );
    preRender->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );
#if( OSGWORKS_OSG_VERSION >= 20906 )
    rootCamera->setImplicitBufferAttachmentMask(
        osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT|osg::Camera::IMPLICIT_DEPTH_BUFFER_ATTACHMENT,
        osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT );
    preCamera->setImplicitBufferAttachmentMask(
        osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT|osg::Camera::IMPLICIT_DEPTH_BUFFER_ATTACHMENT,
        osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT );
#endif

    // Nothing needs to clear. preRender Camera will clear by drawing a fullscreen
    // quad with depth func set to ALWAYS. Disable clearing on all Camera.
    preRender->setClearMask( 0 );
    rootCamera->setClearMask( 0 );
    postRender->setClearMask( 0 );


    // Post-draw callback on root camera handles resolving
    // multisampling for the MRT case.
#ifdef USE_MS
    MSMRTCallback* msmrt = new MSMRTCallback( rootCamera );
    rootCamera->setPostDrawCallback( msmrt );
#endif


    // Configure preRender Camera to draw fullscreen shaded quad.
    // This allows clearing the two color textures to two different colors.
    preRender->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );
    preRender->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    preRender->setRenderOrder( osg::Camera::PRE_RENDER );
    preRender->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );

    preRender->addChild( fsQuad( clearStateSet(), "preRender FSQuad" ) );

    // Configure postRender Camera to draw fullscreen textured quad.
    // This will combine the two (resolved) textures into a single image.
    postRender->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER, osg::Camera::FRAME_BUFFER );
    postRender->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    postRender->setRenderOrder( osg::Camera::POST_RENDER );
    postRender->setComputeNearFarMode( osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR );

    postRender->addChild( fsQuad( combineStateSet( tex0, tex1 ), "postRender FSQuad" ) );
}

int main( int argc, char** argv )
{
    osg::notify( osg::ALWAYS ) <<
// cols:  12345678901234567890123456789012345678901234567890123456789012345678901234567890
         "This is an example of doing multisampled rendering to multiple render targets" << std::endl <<
         "in OSG. It uses osgWorks only to check for OSG version and configure the" << std::endl <<
         "destination textures and RTT Cameras appropriately." << std::endl;

    osg::ArgumentParser arguments( &argc, argv );

    osg::Node* models( osgDB::readNodeFiles( arguments ) );
    if( models == NULL )
    {
        models = osgDB::readNodeFile( "cow.osg" );
        if( models == NULL )
        {
            osg::notify( osg::FATAL ) << "Can't load data files from the command line. Also can't load default \"cow.osg\"." << std::endl;
            return( 1 );
        }
    }
    osg::ref_ptr< osg::Group > root( new osg::Group );
    root->addChild( models );

#ifdef USE_MS
    // Do not unbind the FBOs after the BlitFramebuffer call.
    root->setCullCallback( new KeepFBOsBoundCallback() );
#endif

    // Set fragment program for MRT.
    mrtStateSet( models->getOrCreateStateSet() );


    osgViewer::Viewer viewer;
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    viewer.setUpViewInWindow( 10, 30, winW, winH );
    viewer.setSceneData( root.get() );
    viewer.realize();

    osg::Camera* preRender = new osg::Camera;
    osg::Camera* postRender = new osg::Camera;
    configureCameras( viewer.getCamera(), preRender, postRender );
    root->addChild( preRender );
    root->addChild( postRender );

    osgDB::writeNodeFile( *( viewer.getCamera() ), "out.osg" );



#ifdef USE_MS
    // Required for osgViewer threading model support.
    // Render at least 2 frames before NULLing the cull callback.
    int frameCount( 2 );

    while( !viewer.done() )
    {
        viewer.frame();

        if( frameCount > 0 )
        {
            frameCount--;
            if( frameCount == 0 )
                // After rendering, set cull callback to NULL.
                root->setCullCallback( NULL );
        }
    }
    return( 0 );
#else
    return( viewer.run() );
#endif
}


/** \page msmrt The msmrt Example
msmrt demonstrates rendering multisampled to multiple render targets.

This example demonstrates how to render to multiple multisampled render
targets. Typically, apps require that all multisampled color buffers
get resolved. However, OSG currently inhibits this because it uses a
single glBlitFramebuffer call where multiple calls would be required, one
for each attached color buffer.

To fix this issue, we leverage the Camera post draw callback to execute a
second glBlitFramebuffer call after OSG executes the first blit. The
MSMRTCallback class contains the post-draw callback. The code
assumes two color buffers are attached, and assumes OSG has already resolved
the first color buffer (attachment 0). The code saves the texture attached
as attachment 0, sets it to NULL (leaving only attachment 1), sets the draw
and read buffers to attachment 1, then performs a blit. This resolves
the multisampling in attachment 1. The callback then restores attachment 0.

During development, it was discovered that OSG unbinds the FBOs after it
performs a blit but before it calls the post-draw callback. However, our
callback requires that the FBOs be bound. OSG's RenderStage class can be
configured to keep these FBOs bound, with setDisableFboAfterRender( false ).
It is difficult to call directly into a RenderStage. The only way to execute
this call is with a cull callback, which can query the RenderStage from
the CullVisitor. The KeepFBOsBoundCallback class is responsible
for doing this. In theory, it needs to be done once per cull thread, during
the first frame (for example), then the cull callback can be removed.
However, osgViewer's threading models require this be done for at least
two frames.
*/
