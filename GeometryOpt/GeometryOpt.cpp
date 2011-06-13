//
// Copyright (c) 2009 Skew Matrix Software LLC.
// All rights reserved.
//

#include "CountsVisitor.h"
#include "OptVisitor.h"
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osg/io_utils>
#include <iostream>

void optimizeForDrawElements( osg::Node& root, const float ratioThreshold=.05f )
{
    CountsVisitor cv;
    root.accept( cv );
    cv.dump();
    const float da2Verts( (float)( cv.getDrawArrays() ) / (float)( cv.getVertices() ) );
    osg::notify( osg::INFO ) << "DrawArrays to vertices ratio: " << da2Verts << std::endl;
    if( da2Verts < ratioThreshold )
    {
        osg::notify( osg::INFO ) << "DrawArrays to vertices ratio too small. No optimization." << std::endl;
        return;
    }

    osg::notify( osg::INFO ) << "Converting DrawArrays to DrawElementsUInt." << std::endl;
    OptVisitor ov;
    ov.changeDAtoDEUI_ = true;
    ov.changeDLtoVBO_ = true;
    ov.changeVBOtoDL_ = false;
    ov.changeDynamicToStatic_ = false;
    root.accept( ov );
    ov.dump( osg::notify( osg::ALWAYS ) );
}

void convertToDL( osg::Node& root )
{
    CountsVisitor cv;
    root.accept( cv );
    cv.dump();

    osg::notify( osg::INFO ) << "Converting VBOs to DLs." << std::endl;
    OptVisitor ov;
    ov.changeDAtoDEUI_ = false;
    ov.changeDLtoVBO_ = false;
    ov.changeVBOtoDL_ = true;
    ov.changeDynamicToStatic_ = false;
    root.accept( ov );
    ov.dump( osg::notify( osg::ALWAYS ) );
}

void convertToVBO( osg::Node& root )
{
    CountsVisitor cv;
    root.accept( cv );
    cv.dump();

    osg::notify( osg::INFO ) << "Converting DLs to VBOs." << std::endl;
    OptVisitor ov;
    ov.changeDAtoDEUI_ = false;
    ov.changeDLtoVBO_ = true;
    ov.changeVBOtoDL_ = false;
    ov.changeDynamicToStatic_ = false;
    root.accept( ov );
    ov.dump( osg::notify( osg::ALWAYS ) );
}

int main( int argc, char** argv )
{
    if( argc != 2 )
    {
        osg::notify( osg::FATAL ) << "Must specify input file." << std::endl;
        return( 1 );
    }

    std::string inFile( argv[ 1 ] );
    std::string outFile( "out.ive" );

    osg::ref_ptr< osg::Node > root = osgDB::readNodeFile( inFile );
    if( !root.valid() )
    {
        osg::notify( osg::FATAL ) << "Can't load " << inFile << std::endl;
        return 1;
    }

    //optimizeForDrawElements( *root, 0. );
    //convertToDL( *root );
    convertToVBO( *root );

    osgDB::writeNodeFile( *root, outFile );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.setSceneData( root.get() );
    viewer.run();
}
