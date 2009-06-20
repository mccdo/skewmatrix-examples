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
    ov.changeDLtoVBO_ = false;
    ov.changeDynamicToStatic_ = false;
    root.accept( ov );
    ov.dump( osg::notify( osg::ALWAYS ) );
}

int main()
{
    std::string inFile( "graphicstest.ive" );
    std::string outFile( "out.ive" );

    osg::ref_ptr< osg::Node > root = osgDB::readNodeFile( inFile );
    if( !root.valid() )
        return 1;

    optimizeForDrawElements( *root );

    osgDB::writeNodeFile( *root, outFile );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.setSceneData( root );
    viewer.run();
}
