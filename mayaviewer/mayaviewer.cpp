// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#include "LightManipulator.h"
#include "RenderPrep.h"

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>



int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    osg::ref_ptr< osgDB::ReaderWriter::Options > options = new osgDB::ReaderWriter::Options;
    options->setOptionString( "dds_flip" );

    osg::ref_ptr< osg::Group > root = new osg::Group;
    osg::ref_ptr< osg::Node > models = osgDB::readNodeFiles( arguments, options.get() );
    if( !( models.valid() ) )
    {
        osg::notify( osg::FATAL ) << "Can't open model file(s)." << std::endl;
        return( 1 );
    }

    // Main prep work for rendering.
    RenderPrep renderPrep( models.get() );

    root->addChild( models.get() );


    osg::ref_ptr< LightManipulator > lightManip = new LightManipulator;
    root->addChild( lightManip->getLightSubgraph() );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( lightManip.get() );
    viewer.addEventHandler( new osgViewer::RecordCameraPathHandler );
    viewer.setUpViewInWindow( 10, 30, 800, 450 );
    viewer.setSceneData( root.get() );

    return( viewer.run() );
}
