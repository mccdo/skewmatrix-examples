// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osg/Version>
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


// Define "OSG297" only if the OSG verion is at least v2.9.7.
// This code will also compile on 2.8.2.
#if defined( OSG_MIN_VERSION_REQUIRED )
#  if OSG_MIN_VERSION_REQUIRED( 2,9,7 )
#    define OSG297
#  else
     // OSG_MIN_VERSION_REQUIRED didn't exist until svn head
     // following the 2.9.6 release
#    define OSG297
#  endif
#else
#  undef OSG297
#endif


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
    dpg->setNumPasses( 14 );
    dpg->setMinPasses( 9 );

    dpg->addChild( osgwTools::readNodeFiles(
        "trteapot.osg.(.8).scale.(-1.3,-1.3,0.1).trans trdrawer.osg.90,0,0.rot" ) );
//        "trteapot.osg.(.8).scale.(-1.3,-1.3,0.1).trans trdrawer.osg.90,0,0.rot pliers-big.osg.(.6).scale.-90,0,0.rot.(-.5,-1,0).trans" ) );
//        "trcow.osg.0,10,0.trans trteapot.osg.5.scale.0,-4,0.trans /Projects/temp2/drawer.ive plaincow.osg.10,5,0.trans" ) );
    if( dpg->getNumChildren() == 0 )
        return( 1 );

    root->addChild( dpg.get() );


    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler );
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
#ifdef OSG297
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
