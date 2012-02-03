// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include "ParallelVisitor.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgwMx/MxCore.h>
#include <osg/StateSet>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/TexEnv>
#include <sstream>
#include <map>

struct SourceInfo
{
    SourceInfo()
      : _modelID( 0 ),
        _unit( 0 )
    {}
    SourceInfo( const char& id, const unsigned int& unit )
      : _modelID( id=='a' ? 0 : 1 ),
        _unit( unit )
    {}
    unsigned int _modelID;
    unsigned int _unit;
};
typedef std::map< unsigned int, SourceInfo > UnitMap;

typedef std::vector< osg::ref_ptr< osg::TexEnv > > TexEnvMap;


struct MyParallelCallback : public ParallelVisitor::ParallelVisitorCallback
{
    MyParallelCallback();

    UnitMap _uvMap;
    UnitMap _texMap;
    TexEnvMap _texEnv;

    virtual bool operator()( osg::Node& grpA, osg::Node& grpB );

    void processStateSet( osg::StateSet* ssA, osg::StateSet* ssB );
    void processGeometry( osg::Geometry* geomA, osg::Geometry* geomB );
    void processTexEnv( osg::StateSet* ss, osg::Texture* tex, unsigned int unit );

    typedef std::vector< osg::ref_ptr< osg::Array > > ArrayVec;
    typedef std::vector< osg::ref_ptr< osg::Texture > > TextureVec;
};

MyParallelCallback::MyParallelCallback()
{
}

bool MyParallelCallback::operator()( osg::Node& grpA, osg::Node& grpB )
{
    if( ( grpA.getStateSet() != NULL ) ||
        ( grpB.getStateSet() != NULL ) )
        processStateSet( grpA.getOrCreateStateSet(), grpB.getOrCreateStateSet() );

    if( ( grpA.className() != std::string( "Geode" ) ) ||
        ( grpB.className() != std::string( "Geode" ) ) )
        return( true );
    osg::Geode* geodeA = static_cast< osg::Geode* >( &grpA );
    osg::Geode* geodeB = static_cast< osg::Geode* >( &grpB );

    const unsigned int minDrawables = osg::minimum< unsigned int >( geodeA->getNumDrawables(), geodeB->getNumDrawables() );
    if( geodeA->getNumDrawables() != geodeB->getNumDrawables() )
    {
        osg::notify( osg::WARN ) << "MyParallelCallback: Drawable count mismatch:" << std::endl;
        osg::notify( osg::WARN ) << "\t\"" << geodeA->getName() << "\" " << geodeA->getNumDrawables() << std::endl;
        osg::notify( osg::WARN ) << "\t\"" << geodeB->getName() << "\" " << geodeB->getNumDrawables() << std::endl;
        osg::notify( osg::WARN ) << "\tProcessing the minimum " << minDrawables << "; possible loss of geometry." << std::endl;
    }

    unsigned int idx;
    for( idx=0; idx<minDrawables; idx++ )
    {
        osg::Geometry* geomA = geodeA->getDrawable( idx )->asGeometry();
        osg::Geometry* geomB = geodeB->getDrawable( idx )->asGeometry();
        if( ( geomA != NULL ) && ( geomB != NULL ) )
            processGeometry( geomA, geomB );
    }

    return( true );
}

void MyParallelCallback::processStateSet( osg::StateSet* ssA, osg::StateSet* ssB )
{
    TextureVec ssAtex;
    ssAtex.resize( 16 );
    unsigned int idx;
    for( idx=0; idx<16; idx++ )
        ssAtex[ idx ] = static_cast< osg::Texture* >(
            ssA->getTextureAttribute( idx, osg::StateAttribute::TEXTURE ) );

    UnitMap::const_iterator it;
    for( it = _texMap.begin(); it != _texMap.end(); it++ )
    {
        unsigned int destUnit = it->first;
        osg::Texture* srcTex;
        if( it->second._modelID == 0 )
            srcTex = ssAtex[ it->second._unit ].get();
        else
            srcTex = static_cast< osg::Texture* >(
                ssB->getTextureAttribute( it->second._unit, osg::StateAttribute::TEXTURE ) );
        if( srcTex != NULL )
        {
            ssA->setTextureAttributeAndModes( destUnit, srcTex );
            processTexEnv( ssA, srcTex, destUnit );
        }
    }
}
void MyParallelCallback::processTexEnv( osg::StateSet* ss, osg::Texture* tex, unsigned int unit )
{
    if( unit+1 > _texEnv.size() )
    {
        osg::notify( osg::WARN ) << "MyparallelCallback: processTexEnc: unit " << unit << std::endl;
        osg::notify( osg::WARN ) << "    > _texEnc.size(). Possible missing -e paramter?" << std::endl;
        return;
    }

    osg::TexEnv* te = _texEnv[ unit ].get();
    if( te != NULL )
    {
        ss->setTextureAttribute( unit, te );
        ss->setTextureMode( unit, tex->getTextureTarget(), osg::StateAttribute::ON );
    }
    else
    {
        ss->setTextureMode( unit, tex->getTextureTarget(), osg::StateAttribute::OFF );
    }
}
void MyParallelCallback::processGeometry( osg::Geometry* geomA, osg::Geometry* geomB )
{
    if( ( geomA->getStateSet() != NULL ) ||
        ( geomB->getStateSet() != NULL ) )
        processStateSet( geomA->getOrCreateStateSet(), geomB->getOrCreateStateSet() );

    ArrayVec geomAuv;
    geomAuv.resize( 16 );
    unsigned int idx;
    for( idx=0; idx<16; idx++ )
        geomAuv[ idx ] = geomA->getTexCoordArray( idx );

    UnitMap::const_iterator it;
    for( it = _uvMap.begin(); it != _uvMap.end(); it++ )
    {
        unsigned int destUnit = it->first;
        osg::Array* srcArray;
        if( it->second._modelID == 0 )
            srcArray = geomAuv[ it->second._unit ].get();
        else
            srcArray = geomB->getTexCoordArray( it->second._unit );
        geomA->setTexCoordArray( destUnit, srcArray );
    }
}



// -u 0=a.0 -u 1=a.0 -u 2=b.1
// -t 0=a.0 -t 1=a.1 -t 2=b.1
// -e Diffuse=REPLACE -e Shadow=MODULATE
int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    if( ( arguments.find( "-h" ) > 0 ) || 
        ( arguments.find( "--help" ) > 0 ) )
    {
        osg::notify( osg::ALWAYS ) <<
            "texmerge <fileA> <fileB> [options]\n" <<
            "Texture information from fileB is merged into fileA and written to \"out.osg\".\n" <<
            "Use options to control how texture information merges into the output file:\n" <<
            "\t-u <outUnit>=\"a\"|\"b\".<srcUnit>\n" <<
            "\t-t <outUnit>=\"a\"|\"b\".<srcUnit>\n" <<
            "\t-e <string>=<mode>|\"OFF\"\n" <<
            "\t--view\tIf present, display the resulting model.\n" <<
            "\n" <<
            "\t-u Specify uv texcoord array mappings. Example:\n" <<
            "\t\t\"-u 1=b.0\" means \"take the uv array from fileB's unit 0\n" <<
            "\t\tand write it to output unit 1\n" <<

            "\t-t Specify Texture object mappings. Example:\n" <<
            "\t\t\"-t 0=a.2\" means \"take the Texture object from fileA's\n" <<
            "\t\tunit 2 and write it to output unit 0\n" <<

            "\t-e Specify TexEnv values for texture units. Example:\n" <<
            "\t\t\"-e 1=MODULATE\" means \"set unit 1 to MODULATE. Specify\n" <<
            "\t\t\"OFF\" in place of <mode> to disable a texture.\n" <<
            "\n" <<

            "Typical usage:\n" <<
            "\ttexmerge infile0.osg infile1.osg -u 0=a.0 -u 1=b.0 -t 0=a.0 -t 1=b.0\n" <<
            "\t\t-e 0=REPLACE -e 1=MODULATE -e 2=OFF\n" <<
            std::endl;
        return( 0 );
    }

    MyParallelCallback mpc;

    int uvPos;
    while( ( uvPos = arguments.find( "-u" ) ) > 0 )
    {
        std::istringstream istr( arguments[ uvPos + 1 ] );
        char eq, file, dot;
        unsigned int destUnit, srcUnit;
        istr >> destUnit >> eq >> file >> dot >> srcUnit;

        mpc._uvMap[ destUnit ] = SourceInfo( file, srcUnit );

        arguments.remove( uvPos, 2 );
    }

    int texPos;
    while( ( texPos = arguments.find( "-t" ) ) > 0 )
    {
        std::istringstream istr( arguments[ texPos + 1 ] );
        char eq, file, dot;
        unsigned int destUnit, srcUnit;
        istr >> destUnit >> eq >> file >> dot >> srcUnit;

        mpc._texMap[ destUnit ] = SourceInfo( file, srcUnit );

        arguments.remove( texPos, 2 );
    }

    int texEnvPos;
    while( ( texEnvPos = arguments.find( "-e" ) ) > 0 )
    {
        const std::string str( arguments[ texEnvPos + 1 ] );
        const std::string::size_type loc = str.find( '=' );
        const std::string unitNumber( str.substr( 0, loc ) );
        const std::string modeStr( str.substr( loc+1 ) );

        const unsigned int unit = (unsigned int)( unitNumber[ 0 ] - '0' );

        osg::ref_ptr< osg::TexEnv > te = new osg::TexEnv;
        if( modeStr == std::string( "DECAL" ) ) te->setMode( osg::TexEnv::DECAL );
        else if( modeStr == std::string( "MODULATE" ) ) te->setMode( osg::TexEnv::MODULATE );
        else if( modeStr == std::string( "BLEND" ) ) te->setMode( osg::TexEnv::BLEND );
        else if( modeStr == std::string( "REPLACE" ) ) te->setMode( osg::TexEnv::REPLACE );
        else if( modeStr == std::string( "ADD" ) ) te->setMode( osg::TexEnv::ADD );
        else if( modeStr == std::string( "OFF" ) ) te = NULL;
        else osg::notify( osg::WARN ) << "Unknown mode string: \"" << modeStr << "\"." << std::endl;

        if( unit+1 > mpc._texEnv.size() )
            mpc._texEnv.resize( unit+1 );
        mpc._texEnv[ unit ] = te;

        arguments.remove( texEnvPos, 2 );
    }

    bool view( false );
    int viewPos;
    if( ( viewPos = arguments.find( "--view" ) ) > 0 )
    {
        view = true;
        arguments.remove( viewPos, 1 );
    }

    if( arguments.argc() != 3 )
    {
        osg::notify( osg::FATAL ) << "Must specify two model files on the command line." << std::endl;
        return( 1 );
    }

    osg::ref_ptr< osg::Node > sgA = osgDB::readNodeFile( arguments[ 1 ] );
    osg::ref_ptr< osg::Node > sgB = osgDB::readNodeFile( arguments[ 2 ] );
    if( !( sgA.valid() ) || !( sgB.valid() ) )
    {
        osg::notify( osg::FATAL ) << "Can't load model files." << std::endl;
        return( 1 );
    }

    ParallelVisitor pv( sgA.get(), sgB.get() );
    pv.setCallback( &mpc );
    pv.traverse();

    osgDB::writeNodeFile( *sgA, "out.osg" );

    if( !view )
        return( 0 );


    osg::Group* root = new osg::Group;
    root->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    root->addChild( sgA.get() );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root );
    return( viewer.run() );
}
