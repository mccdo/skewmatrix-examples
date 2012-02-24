// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#include "LightManipulator.h"
#include "RenderPrep.h"

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/AnimationPathManipulator>



int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    int pos;
    float textSize( 0.f );
    if( ( pos = arguments.find( "--textSize" ) ) > 1 )
    {
        arguments.read( pos, "--textSize", textSize );
    }

    float lightSize( 10.f );
    if( ( pos = arguments.find( "--lightSize" ) ) > 1 )
    {
        arguments.read( pos, "--lightSize", lightSize );
    }

    bool parallaxMap( false );
    if( ( pos = arguments.find( "--pm" ) ) > 1 )
    {
        parallaxMap = true;
        arguments.remove( pos, 1 );
    }

    osg::ref_ptr< osgGA::AnimationPathManipulator > apm( NULL );
    std::string animationPathName;
    if( ( pos = arguments.find( "-p" ) ) > 1 )
    {
        arguments.read( pos, "-p", animationPathName );
        const std::string fullName = osgDB::findDataFile( animationPathName );
        if( !( fullName.empty() ) )
            apm = new osgGA::AnimationPathManipulator( fullName );
    }


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
    RenderPrep renderPrep( models.get(), textSize, parallaxMap );

    root->addChild( models.get() );


    osg::ref_ptr< LightManipulator > lightManip = new LightManipulator( lightSize );
    root->addChild( lightManip->getLightSubgraph() );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( lightManip.get() );
    viewer.addEventHandler( new osgViewer::RecordCameraPathHandler );
    if( apm.valid() )
        viewer.setCameraManipulator( apm.get() );
    viewer.setUpViewInWindow( 10, 30, 800, 450 );
    viewer.setSceneData( root.get() );

    return( viewer.run() );
}
