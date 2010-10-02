// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osg/Node>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgwTools/Shapes.h>
#include <osgwTools/Version.h>
#include <osg/OcclusionQueryNode>

#define USE_ISU_CB
#if defined( USE_ISU_CB )
#include "CameraImageCaptureCallback.h"
#else
#include <osgwTools/ScreenCapture.h>
#endif

#include <string>


//
// Begin globalc
const int winW( 800 ), winH( 600 );

osg::ref_ptr< osg::Texture2D > tex;

osg::ref_ptr< osg::Camera > preRenderCamera;

#if defined( USE_ISU_CB )
std::string imageDumpName( "out.png" );
osg::ref_ptr< CameraImageCaptureCallback > captureCB;
#else
osg::ref_ptr< osgwTools::ScreenCapture > captureCB;
#endif

// End globals
//



class Handler : public osgGA::GUIEventHandler 
{
public: 
    Handler() : _frameCount( -1 ) {}
    ~Handler() {}

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        bool handled( false );
        switch(ea.getEventType())
        {
            case( osgGA::GUIEventAdapter::FRAME ):
            {
#if defined( USE_ISU_CB )
                if( _frameCount > -1 )
                {
                    if( --_frameCount == 0 )
                        preRenderCamera->setPostDrawCallback( NULL );
                }
#endif
            }
            case( osgGA::GUIEventAdapter::KEYDOWN ):
            {
                switch( ea.getKey() )
                {
                    case( osgGA::GUIEventAdapter::KEY_Return ):
                    {
#if defined( USE_ISU_CB )
                        preRenderCamera->setPostDrawCallback( captureCB.get() );
                        _frameCount = 2;
#else
                        captureCB->setNumFramesToCapture( 1 );
                        captureCB->setCapture( true );
#endif
                        handled = true;
                    }
                }
            }
        }
        return( handled );
    }

    int _frameCount;
};


osg::Node*
preRender( osg::Group* root, osg::Node* model )
{
    // Create the texture; we'll use this as our color buffer.
    // Note it has no image data; not required.
    tex = new osg::Texture2D;
    tex->setTextureWidth( winW );
    tex->setTextureHeight( winH );
    tex->setInternalFormat( GL_RGBA );
    tex->setBorderWidth( 0 );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
#if defined( USE_ISU_CB )
    osg::Image* image = new osg::Image;
    tex->setImage( image );
#endif


    // Configure preRenderCamera to draw fullscreen textured quad
    preRenderCamera = new osg::Camera;
    preRenderCamera->setClearColor( osg::Vec4( .5, 0., 0., 1. ) );

    preRenderCamera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    preRenderCamera->setRenderOrder( osg::Camera::PRE_RENDER );
    preRenderCamera->setViewMatrix( osg::Matrixd::translate( 0., 0., -30. ) );
    preRenderCamera->setProjectionMatrix( osg::Matrixd::perspective( 25., 1., 1., 100. ) );

    preRenderCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );
    preRenderCamera->attach( osg::Camera::COLOR_BUFFER0, tex.get(), 0, 0, false );

#if 0
    osg::OcclusionQueryNode* oqn = new osg::OcclusionQueryNode;
    oqn->addChild( model );
    preRenderCamera->addChild( oqn );
#else
    preRenderCamera->addChild( model );
#endif

#if defined( USE_ISU_CB )
    captureCB = new CameraImageCaptureCallback( imageDumpName, 512, 512, tex );
#else
    captureCB = new osgwTools::ScreenCapture;
    preRenderCamera->setPostDrawCallback( captureCB.get() );
#endif


    // Configure textured tri pair to hang off the root node and
    // display our texture.
    osg::Geode* geode( new osg::Geode );
    geode->addDrawable( osgwTools::makePlane(
        osg::Vec3( -1,0,-1 ), osg::Vec3( 2,0,0 ), osg::Vec3( 0,0,2 ) ) );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(
        0, tex, osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    root->addChild( geode );


    return( preRenderCamera.get() );
}

int
main( int argc, char** argv )
{
    // Disable serialization of draw threads.
    // TBD verify that OSG picks this up.
    putenv( "OSG_SERIALIZE_DRAW_DISPATCH=OFF" );

    osg::ref_ptr< osg::Group > root( new osg::Group );
    osg::ref_ptr< osg::Node > model( osgDB::readNodeFile( "cow.osg" ) );
    if( !model.valid() )
        return( 1 );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.realize();
    viewer.addEventHandler( new Handler );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::ThreadingHandler );

    root->addChild( preRender( root.get(), model.get() ) );

    viewer.getCamera()->setClearColor( osg::Vec4( 0.4, 0.4, 0.4, 1. ) );

    return( viewer.run() );
}
