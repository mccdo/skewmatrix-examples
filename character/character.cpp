// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgwTools/Shapes.h>
#include <osg/PolygonMode>

#include "CharacterFixVisitor.h"

#include <string>
#include <iostream>

int main( int argc, char** argv )
{
    // Specific to my computer.
    std::string ioPath( "C:/Projects/animation/US_A5_Walk_sideways" );
    osgDB::Registry::instance()->getDataFilePathList().push_front( ioPath );

//#define USE_COW
#ifdef USE_COW
    std::string filename( "cow.osg" );
#else
    //std::string filename( "US_A5_Walk_sideways_Feet_Cross_over_v01.fbx" );
    std::string filename( "US_A5_Walk_sideways_Foot_to_Foot_v01.fbx" );
#endif

    osg::ref_ptr< osg::Group > group( new osg::Group );
    osg::Node* model( osgDB::readNodeFile( filename ) );
    if( model == NULL )
        return( 1 );
    group->addChild( model );

    CharacterFixVisitor cfv;
#ifdef USE_COW
    cfv.setTexturePathControl( false );
    cfv.setScaleFactor( 0.5 );
    cfv.setReverseNormals( false );
#else
    cfv.setTexturePathControl( true, "Images/" );
    // Use default scaling, cm->ft
    // Reverse the normals. For some reason, they are backwards.
#endif
    model->accept( cfv );

    // Output to .osg.
    osgDB::writeNodeFile( *model, ioPath + std::string( "/out.osg" ) );

    // Add axes
    osg::Node* axes( osgDB::readNodeFile( "axes.osg" ) );
    if( axes != NULL )
        group->addChild( axes );

    // Add plan / grid
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable( osgwTools::makePlane( osg::Vec3( 0., 0., 0. ),
            osg::Vec3( 3., 0., 0. ), osg::Vec3( 0., 6., 0. ), osg::Vec2s( 3, 6 ) ) );
        geode->addDrawable( osgwTools::makePlane( osg::Vec3( 0., 0., 0. ),
            osg::Vec3( 3., 0., 0. ), osg::Vec3( 0., 0., 2. ), osg::Vec2s( 3, 2 ) ) );
        group->addChild( geode );

        osg::StateSet* ss( new osg::StateSet );
        ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        ss->setAttributeAndModes( new osg::PolygonMode(
            osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE ) );
        geode->setStateSet( ss );
    }

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
    viewer.setSceneData( group.get() );
    return( viewer.run() );
}
