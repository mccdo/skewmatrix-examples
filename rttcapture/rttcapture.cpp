// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osg/Node>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgwTools/Version.h>
#include <osgwTools/ScreenCapture.h>

#include <string>


const int winW( 800 ), winH( 600 );

osg::ref_ptr< osg::Texture2D > tex;

osg::ref_ptr< osgwTools::ScreenCapture > captureCB;


class Handler : public osgGA::GUIEventHandler 
{
public: 
    Handler() {}
    ~Handler() {}

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        bool handled( false );
        switch(ea.getEventType())
        {
            case( osgGA::GUIEventAdapter::KEYDOWN ):
            {
                switch( ea.getKey() )
                {
                    case( osgGA::GUIEventAdapter::KEY_Return ):
                    {
                        captureCB->setNumFramesToCapture( 1 );
                        captureCB->setCapture( true );
                        handled = true;
                    }
                }
            }
        }
        return( handled );
    }
};


osg::Node*
postRender( osgViewer::Viewer& viewer )
{
    osg::Camera* rootCamera( viewer.getCamera() );

    // Create the texture; we'll use this as our color buffer.
    // Note it has no image data; not required.
    tex = new osg::Texture2D;
    tex->setTextureWidth( winW );
    tex->setTextureHeight( winH );
    tex->setInternalFormat( GL_RGBA );
    tex->setBorderWidth( 0 );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );

    // Attach the texture to the camera. Tell it to use multisampling.
    // Internally, OSG allocates a multisampled renderbuffer, renders to it,
    // and at the end of the frame performs a BlitFramebuffer into our texture.
    rootCamera->attach( osg::Camera::COLOR_BUFFER0, tex.get(), 0, 0, false );
    rootCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );

    captureCB = new osgwTools::ScreenCapture;
    rootCamera->setPostDrawCallback( captureCB.get() );


    // Configure postRenderCamera to draw fullscreen textured quad
    osg::ref_ptr< osg::Camera > postRenderCamera( new osg::Camera );
    postRenderCamera->setClearColor( osg::Vec4( 0., 1., 0., 1. ) ); // should never see this.
    postRenderCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER, osg::Camera::FRAME_BUFFER );

    postRenderCamera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    postRenderCamera->setRenderOrder( osg::Camera::POST_RENDER );
    postRenderCamera->setViewMatrix( osg::Matrixd::identity() );
    postRenderCamera->setProjectionMatrix( osg::Matrixd::identity() );

    osg::Geode* geode( new osg::Geode );
    geode->addDrawable( osg::createTexturedQuadGeometry(
        osg::Vec3( -1,-1,0 ), osg::Vec3( 2,0,0 ), osg::Vec3( 0,2,0 ) ) );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(
        0, tex, osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    postRenderCamera->addChild( geode );

    return( postRenderCamera.release() );
}

int
main( int argc, char** argv )
{
    osg::ref_ptr< osg::Group > root( new osg::Group );
    root->addChild( osgDB::readNodeFile( "cow.osg" ) );
    if( root->getNumChildren() == 0 )
        return( 1 );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, winW, winH );
    viewer.setSceneData( root.get() );
    viewer.realize();
    viewer.addEventHandler( new Handler );

    root->addChild( postRender( viewer ) );

    // Clear to white to make AA extremely obvious.
    viewer.getCamera()->setClearColor( osg::Vec4( 1., 1., 1., 1. ) );

    return( viewer.run() );
}
