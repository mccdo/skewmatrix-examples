// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#include "LightManipulator.h"
#include "RenderPrep.h"

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/StateSetManipulator>



int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    if( arguments.read( "-h" ) ||
        arguments.read( "-?" ) )
    {
        OSG_NOTICE << "Command line arguments:" << std::endl;
        OSG_NOTICE << "  --textSize <f>\tSet the (floating point) text size to <f>. Default: 0.0 (no text)." << std::endl;
        OSG_NOTICE << "  --lightSize <f>\tSet the (floating point) light sphere radius to <f>. Default: 10.0." << std::endl;
        OSG_NOTICE << "  --pm\t\t\tEnable parallax mapping (bump maps required). Default: Parallax mapping disabled." << std::endl;
        OSG_NOTICE << "  -p <file>\t\tLoad and execute the specified animation path file (as in osgviewer)." << std::endl;
        OSG_NOTICE << "Runtime keyboard commands:" << std::endl;
        OSG_NOTICE << "  ctrl a, ctrl s\tMove light in -x and +x." << std::endl;
        OSG_NOTICE << "  ctrl d, ctrl f\tMove light in -y and +y." << std::endl;
        OSG_NOTICE << "  ctrl e, ctrl c\tMove light in -z and +z." << std::endl;
        OSG_NOTICE << "  -, +\t\t\tDecrease and increase the delta light motion." << std::endl;
        OSG_NOTICE << "  Record camera animation path:" << std::endl;
        OSG_NOTICE << "    z\t\t\tRecord camera path to 'saved_animation.path'." << std::endl;
        OSG_NOTICE << "  Statistics:" << std::endl;
        OSG_NOTICE << "    s\t\t\tCycle performance statistics." << std::endl;
        OSG_NOTICE << "  Window size:" << std::endl;
        OSG_NOTICE << "    f\t\t\tToggle fullscreen/windowed." << std::endl;
        OSG_NOTICE << "  StateSet modification:" << std::endl;
        OSG_NOTICE << "    b\t\t\tToggle backface culling." << std::endl;
        OSG_NOTICE << "    w\t\t\tCycle fill/wire/point polygon mode." << std::endl;
        return( 0 );
    }

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


    osg::ref_ptr< LightManipulator > lightManip( new LightManipulator( lightSize ) );
    root->addChild( lightManip->getLightSubgraph() );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( lightManip.get() );
    viewer.addEventHandler( new osgViewer::RecordCameraPathHandler() );
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler() );
    viewer.addEventHandler( new osgGA::StateSetManipulator(
        viewer.getCamera()->getOrCreateStateSet() ) );
    if( apm.valid() )
        viewer.setCameraManipulator( apm.get() );
    viewer.setSceneData( root.get() );

    return( viewer.run() );
}
