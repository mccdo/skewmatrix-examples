// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osgwTools/Version.h>
#include <osg/Node>
#include <osg/Group>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgwTools/ReadFile.h>
#include <osgDB/FileUtils>

#include "DepthPeelGroup.h"
#include "DepthPeelRenderStage.h"

#include <string>



osg::StateSet*
depthPeelState( int unit )
{
    osg::ref_ptr< osg::StateSet > ss = new osg::StateSet;

    osg::Shader* vertShader = new osg::Shader( osg::Shader::VERTEX );
    vertShader->loadShaderSourceFromFile( osgDB::findDataFile( "DepthPeelCompare.vs" ) );
    osg::Shader* fragShader = new osg::Shader( osg::Shader::FRAGMENT );
    fragShader->loadShaderSourceFromFile( osgDB::findDataFile( "DepthPeelCompare.fs" ) );

    osg::Program* program = new osg::Program();
    program->addShader( vertShader );
    program->addShader( fragShader );
    ss->setAttribute( program, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    ss->addUniform( new osg::Uniform( "depthPeelDepthMap", unit ) );

    return( ss.release() );
}


int
main( int argc, char** argv )
{
    //osg::setNotifyLevel( osg::DEBUG_FP );

    osg::ref_ptr< osg::Group > root( new osg::Group );

    osg::ref_ptr< DepthPeelGroup > dpg( new DepthPeelGroup );
    dpg->setTextureUnit( 3 );
    dpg->setStateSet( depthPeelState( dpg->getTextureUnit() ) );
    dpg->setNumPasses( 16 );
    dpg->setMinPasses( 16 );

    dpg->addChild( osgwTools::readNodeFiles(
//        "trteapot.osg.(.8).scale.(-1.3,-1.3,0.1).trans trdrawer.osg.90,0,0.rot" ) );
        "trteapot.osg.(.8).scale.(-1.3,-1.3,0.1).trans trdrawer.osg.90,0,0.rot pliers-big.osg.(.6).scale.-90,0,0.rot.(-.5,-1,0).trans" ) );
//        "trcow.osg.0,10,0.trans trteapot.osg.5.scale.0,-4,0.trans /Projects/temp2/drawer.ive plaincow.osg.10,5,0.trans" ) );
    if( dpg->getNumChildren() == 0 )
        return( 1 );

    root->addChild( dpg.get() );


    osgViewer::Viewer viewer;
    viewer.setUpViewOnSingleScreen( 0 );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::ThreadingHandler );
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setSceneData( root.get() );

    // TBD
    // We really need to set the clear color as a uniform.
    viewer.getCamera()->setClearColor( osg::Vec4( 0.1, 0.1, 0.1, 1. ) );

    // TBD
    // Hm, how do we want to handle this? Ultimately, DPG will
    // require FBO usage, and top-level camera will draw a full-screen
    // quad. DPRS will pick up clear color from top-level camera,
    // but maybe it should ignore top-level clear mask.
    // For now, must set clear mask to 0 so that top-level camera
    // doesn't clear after DPRS draws.
    viewer.getCamera()->setClearMask( 0 );
#if( OSGWORKS_OSG_VERSION > 20907 )
    // Version is 2.9.7 or greater; master camera clear mask shouls automatically
    // propogate down to slave cameras.
#else
    // OSG doesn't inherit clear mask to slave Cameras prior to 2.9.7, so after a call
    // to realize(), loop over slaves and set clear mask explicitly.
    viewer.realize();
    unsigned int idx;
    for( idx=0; idx<viewer.getNumSlaves(); idx++ )
        viewer.getSlave( idx )._camera->setClearMask( 0 );
#endif

    return( viewer.run() );
}
