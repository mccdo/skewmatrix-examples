// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/FileNameUtils>
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
    if( argc != 2 )
    {
        osg::notify( osg::FATAL ) << "Usage: character <filename>" << std::endl;
        return( 1 );
    }

    const std::string inFile( argv[ 1 ] );
    std::string filePath( osgDB::getFilePath( inFile ) );
    if( filePath.empty() )
    {
        filePath = ".";
    }
    const std::string baseName( osgDB::getStrippedName( inFile ) );
    const std::string outFile( filePath + "/" + baseName + ".osg" );
    osg::notify( osg::DEBUG_INFO ) << "In: " << inFile << std::endl;
    osg::notify( osg::DEBUG_INFO ) << "Out: " << outFile << std::endl;

    osg::ref_ptr< osg::Group > group( new osg::Group );
    osg::Node* model( osgDB::readNodeFile( inFile ) );
    if( model == NULL )
        return( 1 );

    CharacterFixVisitor cfv;
    const bool cowConfig = ( baseName == std::string( "cow" ) );
    if( cowConfig ) // Just for testing.
    {
        cfv.setTexturePathControl( false );
        cfv.setReverseNormals( false );
    }
    else
    {
        cfv.setTexturePathControl( true, "Images/" );
        // Use default scaling, cm->ft
        // Reverse the normals. For some reason, they're backwards.
    }
    osg::Node* processedModel = cfv.process( *model );
    group->addChild( processedModel );

    // Output to .osg.
    osgDB::writeNodeFile( *processedModel, outFile );

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
