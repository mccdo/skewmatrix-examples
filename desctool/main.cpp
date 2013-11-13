// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "RemoveByDesc.h"

#include <iostream>


int main( int argc, char** argv )
{
    if( argc < 2 )
    {
        std::cerr << "desctool <infile> [<outfile>]" << std::endl;
        exit( 1 );
    }

    osg::ref_ptr< osg::Node > scene( osgDB::readNodeFile( argv[ 1 ] ) );

    osg::Node::DescriptionList criteria;
    criteria.push_back( "NUGRAF___AccountedCounter" );
    //criteria.push_back( "AccountedCounter" );
    criteria.push_back( "ok_int: -1" );

    RemoveByDesc rbd;
    rbd.setDescriptions( criteria );
    unsigned int count( rbd.execute( scene.get() ) );
    std::cout << "Removed " << count << " nodes." << std::endl;

    std::string filename = "output.ive";
    if( argc > 2 )
    {
        filename = argv[ 2 ];
    }
    osgDB::writeNodeFile( *scene, filename );
    return 0;
}
