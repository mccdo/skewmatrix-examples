// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "ShareNodes.h"

#include <iostream>


int main( int argc, char** argv )
{
    if( argc < 2 )
    {
        std::cerr << "nodeshare <infile> [<outfile>]" << std::endl;
        exit( 1 );
    }


    osg::ref_ptr< osg::Node > scene( osgDB::readNodeFile( argv[ 1 ] ) );

    ShareNodes snv;
    snv.execute( scene.get() );

    std::string filename = "output.ive";
    if( argc > 2 )
        filename = argv[ 2 ];
    osgDB::writeNodeFile( *scene, filename );
}
