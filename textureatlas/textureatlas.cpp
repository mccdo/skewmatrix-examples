// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgwTools/CountsVisitor.h>
#include <osgUtil/Optimizer>


int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    osg::ref_ptr< osg::Group > root = new osg::Group;
    root->addChild( osgDB::readNodeFiles( arguments ) );
    if( root->getNumChildren() == 0 )
    {
        osg::notify( osg::FATAL ) << "No data loaded." << std::endl;
        return( 1 );
    }

    osgwTools::CountsVisitor counts;
    root->accept( counts );
    counts.dump( osg::notify( osg::ALWAYS ) );

    osgUtil::Optimizer opt;
    unsigned int flags = osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER |
        osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS;
    opt.optimize( root.get(), flags );

    counts.reset();
    root->accept( counts );
    counts.dump( osg::notify( osg::ALWAYS ) );

    return( 0 );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return( viewer.run() );
}