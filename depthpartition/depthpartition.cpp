// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osg/Node>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <string>


const int winW( 800 ), winH( 600 );


osg::Node*
createDepthPartitionGeometry()
{
    osg::ref_ptr< osg::Group > root = new osg::Group;
    root->addChild( osgDB::readNodeFile( "cow.osg" ) );

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable( osg::createTexturedQuadGeometry(
        osg::Vec3( -10., 5., -10 ), osg::Vec3( 20, 0, 0 ), osg::Vec3( 0, 0, 20 ) ) );
    geode->addDrawable( osg::createTexturedQuadGeometry(
        osg::Vec3( -200000., -400000., 0 ), osg::Vec3( 400000, 0, 0 ), osg::Vec3( 0, 400000, 0 ) ) );
    root->addChild( geode );

    return( root.release() );
}

int
main( int argc, char** argv )
{
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, winW, winH );
    viewer.setSceneData( createDepthPartitionGeometry() );
    // Set FOV to 0.0025 degrees (very narrow) to zoom in on distant cow.
    // near/far are irrelevant, as OSG auto computes them.
    viewer.getCamera()->setProjectionMatrixAsPerspective( 0.0025, 1.333, 1., 10. );

    osgGA::TrackballManipulator* tb = new osgGA::TrackballManipulator;
    tb->setHomePosition( osg::Vec3( 0., -400025., 0.),
        osg::Vec3( 0, 0, 0), osg::Vec3( 0., 0., 1.) );
    viewer.setCameraManipulator( tb );

    while( !viewer.done() )
    {
        {
            double fovy, aspect, znear, zfar;
            viewer.getCamera()->getProjectionMatrixAsPerspective( fovy, aspect, znear, zfar );
            osg::notify( osg::ALWAYS ) << "Near: " << znear << ",   Far: " << zfar << ",  Ratio: " << zfar/znear << std::endl;
        }
        viewer.frame();
    }
    return( 0 );
}
