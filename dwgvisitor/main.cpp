// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osg/MatrixTransform>
#include <osgwTools/Shapes.h>

#include "RemoveNodeNameVisitor.h"

int
main( int argc, char** argv )
{
    osg::ref_ptr< osg::Node > scene = osgDB::readNodeFile( argv[ 1 ] );

    ves::xplorer::scenegraph::util::RemoveNodeNameVisitor( scene.get(), "", "" );

    osgDB::writeNodeFile( *scene.get(), "output.osg" );
}
