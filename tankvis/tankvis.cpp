// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osg/io_utils>

#include "TankData.h"



int main( int argc, char** argv )
{
    if( argc == 1 )
    {
        osg::notify( osg::FATAL ) << "Please specify a model." << std::endl;
        return( 1 );
    }
    osg::ArgumentParser arguments( &argc, argv );

    osg::Vec3 up( 0., 0., 1. );
    float x, y, z;
    if( arguments.read( "-up", x, y, z ) )
        up.set( x, y, z );
    osg::notify( osg::NOTICE ) << "  Up: " << up << std::endl;

    osg::Node* models = osgDB::readNodeFiles( arguments );

    TankData* td = new TankData( models );
    td->setColorMethod( TankData::COLOR_EXPLICIT );
    td->setExplicitColor( osg::Vec4( 1., .5, .1, .8 ) );
    td->setPercentOfCapacity( 0.6f );

    osgViewer::Viewer viewer;
    viewer.setSceneData( models );

    viewer.run();

    delete td;
}
