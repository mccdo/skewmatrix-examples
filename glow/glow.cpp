// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osg/Node>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/CompositeViewer>
#include <osgGA/TrackballManipulator>

#include <string>


const int winW( 800 ), winH( 600 );


osgViewer::View*
renderPipe( osgGA::TrackballManipulator* tb, osg::Node* root, int x, int y )
{
    osg::ref_ptr< osgViewer::View > view = new osgViewer::View;
    view->setUpViewInWindow( x, y, winW, winH );
    view->getCamera()->setClearColor( osg::Vec4( 0., 0., 0., 1. ) );
    view->getCamera()->setViewMatrix( osg::Matrix::identity() );
    view->getCamera()->setProjectionMatrix( osg::Matrix::identity() );

    view->addEventHandler( tb );
    tb->setNode( root );
    tb->home( 0 );

    osg::Camera* rootCamera = new osg::Camera;
    rootCamera->setProjectionMatrixAsPerspective( 40., (double)winW / (double)winH, 1., 100. );
    rootCamera->addChild( root );

    // Create the texture; we'll use this as our color buffer.
    // Note it has no image data; not required.
    osg::Texture2D* tex = new osg::Texture2D;
    tex->setTextureWidth( winW );
    tex->setTextureHeight( winH );
    tex->setInternalFormat( GL_RGBA );
    tex->setBorderWidth( 0 );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );

    rootCamera->attach( osg::Camera::COLOR_BUFFER0, tex, 0, 0, false );
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

    rootCamera->addChild( postRenderCamera.get() );
    view->setSceneData( rootCamera );

    return( view.release() );
}

int
main( int argc, char** argv )
{
    osg::ref_ptr< osg::Group > root( new osg::Group );

    std::vector< std::string > files;
    files.push_back( "dumptruck.osg" );
    files.push_back( "cow.osg.-4,18,0.trans" );
    root->addChild( osgDB::readNodeFiles( files ) );
    if( root->getNumChildren() == 0 )
        return( 1 );

    osgGA::TrackballManipulator* tb = new osgGA::TrackballManipulator;
    osgViewer::CompositeViewer viewer;
    viewer.addView( renderPipe( tb, root.get(), 10, 30 ) );
    viewer.addView( renderPipe( tb, root.get(), 10+winW, 30 ) );
    viewer.addView( renderPipe( tb, root.get(), 10, 30+winH ) );
    viewer.addView( renderPipe( tb, root.get(), 10+winW, 30+winH ) );

    while( !viewer.done() )
    {
        int idx;
        for( idx=0; idx<4; idx++ )
            viewer.getView( idx )->getCamera()->setViewMatrix( tb->getInverseMatrix() );

        viewer.frame();
    }
}
