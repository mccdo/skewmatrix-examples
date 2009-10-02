// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osg/Node>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <string>


const int winW( 800 ), winH( 600 );


osg::Node*
postRender( osgViewer::Viewer& viewer )
{
    osg::Camera* rootCamera( viewer.getCamera() );

    // Create the texture; we'll use this as our color buffer.
    // Note it has no image data; not required.
    osg::Texture2D* tex = new osg::Texture2D;
    tex->setTextureWidth( winW );
    tex->setTextureHeight( winH );
    tex->setInternalFormat( GL_RGBA );
    tex->setBorderWidth( 0 );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );

    // Attach the texture to the camera. Tell it to use multisampling.
    // Internally, OSG allocates a multisampled renderbuffer, renders to it,
    // and at the end of the frame performs a BlitFramebuffer into our texture.
    rootCamera->attach( osg::Camera::COLOR_BUFFER0, tex, 0, 0, false, 8, 8 );
    rootCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );


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
    osg::setNotifyLevel( osg::INFO );

    osg::ref_ptr< osg::Group > root( new osg::Group );
    root->addChild( osgDB::readNodeFile( "cow.osg" ) );
    if( root->getNumChildren() == 0 )
        return( 1 );

    osgViewer::Viewer viewer;
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 10, 30, winW, winH );
    viewer.setSceneData( root.get() );
    viewer.realize();

    root->addChild( postRender( viewer ) );

    // Clear to white to make AA extremely obvious.
    viewer.getCamera()->setClearColor( osg::Vec4( 1., 1., 1., 1. ) );

    return( viewer.run() );
}
