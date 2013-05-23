
#include "MipMapLimiter.h"
#include "UnRefImageDataVisitor.h"

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgViewer/Viewer>

#include <iostream>

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    std::string output;
    arguments.read( "--out", output );
    if( output.empty() )
        output = std::string( "out.ive" );

    int maxDim( 256 );
    arguments.read( "--maxDim", maxDim );

    bool outputTextures = arguments.read( "--outputTextures" );

    osg::Node* root = osgDB::readNodeFiles( arguments );
    if( root == NULL )
    {
        osg::notify( osg::FATAL ) << "Can't load data file." << std::endl;
        return( 1 );
    }
    
    if( outputTextures )
    {
        ves::xplorer::scenegraph::util::UnRefImageDataVisitor unrefImage( root );       
    }

    osgViewer::Viewer viewer;
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 10, 30, 768, 480 );
    viewer.setSceneData( root );

    viewer.run();

    // Super hack, but this is what we get from osgViewer, I guess.
    // The MipMapLimiter NodeVisitor needs an unsigned int context ID.
    // osgViewer has one, but there's no (obvious) way to get to it
    // outside of the draw (and sometimes cull) traversals.
    // Fortunately, we can assume a value of '0' for the context ID if
    // the number of contexts is 1, which it should be because we used
    // SingleThreaded and draw into a single window on a single screen.
    osgViewer::ViewerBase::Contexts ctxts;
    viewer.getContexts( ctxts );
    if( ctxts.size() != 1 )
    {
        osg::notify( osg::ALWAYS ) << "Error: context vector size " << ctxts.size() << std::endl;
        osg::notify( osg::ALWAYS ) << "Must have size==1." << ctxts.size() << std::endl;
        return( 1 );
    }
    const unsigned int contextID( 0 );


    osgwTools::MipMapLimiter mml( contextID );
    mml.setLimitModeAndValue( osgwTools::MipMapLimiter::MAX_DIMENSION, maxDim );
    mml.setTextureIOFlag( outputTextures );

    root->accept( mml );
    osg::notify( osg::ALWAYS ) << "Total # textures: " << mml.totalAllTextures << std::endl;
    if( mml.getLimitMode() == osgwTools::MipMapLimiter::MAX_DIMENSION )
        osg::notify( osg::ALWAYS ) << "Total textures reduced: " << mml.totalTexturesExceedingMaxDimension << std::endl;
    osg::notify( osg::ALWAYS ) << "Total unsupported: " << mml.totalUnsupported << std::endl;

    //do NOT embed the images in the ive
    if( outputTextures )
    {
        osg::ref_ptr< osgDB::ReaderWriter::Options > noImgOpt = 
            new osgDB::ReaderWriter::Options();
        noImgOpt->setOptionString( "noTexturesInIVEFile" );
        osgDB::Registry::instance()->setOptions( noImgOpt.get() );
    }
    
    osgDB::writeNodeFile( *root, output );
    osg::notify( osg::ALWAYS ) << "Output written to " << output << std::endl;

    return( 0 );
}
