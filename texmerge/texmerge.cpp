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

typedef std::map< std::string, osg::ref_ptr< osg::TexEnv > > TexEnvMap;


struct MyParallelCallback : public ParallelVisitor::ParallelVisitorCallback
{
    MyParallelCallback();

    bool _searchObjName;

    UnitMap _uvMap;
    UnitMap _texMap;
    TexEnvMap _texEnvMap;

    virtual bool operator()( osg::Node& grpA, osg::Node& grpB );

    void processStateSet( osg::StateSet* ssA, osg::StateSet* ssB );
    void processGeometry( osg::Geometry* geomA, osg::Geometry* geomB );
    void processTexEnv( osg::StateSet* ss, osg::Texture* tex, unsigned int unit );

    typedef std::vector< osg::ref_ptr< osg::Array > > ArrayVec;
    typedef std::vector< osg::ref_ptr< osg::Texture > > TextureVec;
};

MyParallelCallback::MyParallelCallback()
  : _searchObjName( false )
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

    const unsigned int numDrawables = osg::minimum< unsigned int >( geodeA->getNumDrawables(), geodeB->getNumDrawables() );
    unsigned int idx;
    for( idx=0; idx<numDrawables; idx++ )
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
    std::string texName;
    osg::Image* image = tex->getImage( 0 ); // cubemap face number.
    if( image != NULL )
        texName = image->getFileName();
    if( _searchObjName || texName.empty() )
        texName = tex->getName();

    TexEnvMap::const_iterator envIt;
    for( envIt = _texEnvMap.begin(); envIt != _texEnvMap.end(); envIt++ )
    {
        if( texName.find( envIt->first ) != texName.npos )
        {
            if( envIt->second.valid() )
            {
                ss->setTextureAttribute( unit, envIt->second.get() );
                ss->setTextureMode( unit, tex->getTextureTarget(), osg::StateAttribute::ON );
            }
            else
                ss->setTextureMode( unit, tex->getTextureTarget(), osg::StateAttribute::OFF );
        }
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
            "\t--objName\n" <<
            "\n" <<
            "\t-u Specify uv texcoord array mappings. Example:\n" <<
            "\t\t\"-u 1=b.0\" means \"take the uv array from fileB's unit 0\n" <<
            "\t\tand write it to output unit 1\n" <<

            "\t-t Specify Texture object mappings. Example:\n" <<
            "\t\t\"-u 0=a.2\" means \"take the Texture object from fileA's\n" <<
            "\t\tunit 2 and write it to output unit 0\n" <<

            "\t-e Specify TexEnv values for matched search strings. Example:\n" <<
            "\t\t\"-e VES_Shadow=MODULATE\" means \"if the texture file name\n" <<
            "\t\tcontains 'VES_Shadow', store a TexEnv MODULATE object in the\n" <<
            "\t\tsame StateSet and texture unit as that texture. Specify \"OFF\"\n" <<
            "\t\tin place of <mode> to disable a texture.\n" <<

            "\t--objName If present, texmerge serarches for the \"-e\" search string\n" <<
            "\t\tin the osg::Texture Object name. By default, texmerge\n" <<
            "\t\tsearches in the osg::Texture's Image::getFileName() string.\n" <<
            "\n" <<

            "Typical usage:\n" <<
            "\ttexmerge infile0.osg infile1.osg -u 0=a.0 -u 1=b.0 -t 0=a.0 -t 1=b.0\n" <<
            "\t\t-e VES_Diffuse=REPLACE -e VES_Shadow=MODULATE -e VES_Normal=OFF\n" <<
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
        const std::string objectName( str.substr( 0, loc ) );
        const std::string modeStr( str.substr( loc+1 ) );

        osg::ref_ptr< osg::TexEnv > te = new osg::TexEnv;
        if( modeStr == std::string( "DECAL" ) ) te->setMode( osg::TexEnv::DECAL );
        else if( modeStr == std::string( "MODULATE" ) ) te->setMode( osg::TexEnv::MODULATE );
        else if( modeStr == std::string( "BLEND" ) ) te->setMode( osg::TexEnv::BLEND );
        else if( modeStr == std::string( "REPLACE" ) ) te->setMode( osg::TexEnv::REPLACE );
        else if( modeStr == std::string( "ADD" ) ) te->setMode( osg::TexEnv::ADD );
        else if( modeStr == std::string( "OFF" ) ) te = NULL;
        else osg::notify( osg::WARN ) << "Unknown mode string: \"" << modeStr << "\"." << std::endl;

        mpc._texEnvMap[ objectName ] = te;

        arguments.remove( texEnvPos, 2 );
    }

    if( arguments.find( "--objName" ) > 0 )
    {
        mpc._searchObjName = true;
        arguments.remove( arguments.find( "--objName" ) );
    }

    if( arguments.argc() != 3 )
    {
        osg::notify( osg::FATAL ) << "Must specify two model files on the command line." << std::endl;
        return( 1 );
    }

    osg::ref_ptr< osg::Node > sgA = osgDB::readNodeFile( arguments[ 1 ] );
    osg::ref_ptr< osg::Node > sgB = osgDB::readNodeFile( arguments[ 2 ] );

    ParallelVisitor pv( sgA.get(), sgB.get() );
    pv.setCallback( &mpc );
    pv.traverse();

    osgDB::writeNodeFile( *sgA, "out.osg" );

    osg::Group* root = new osg::Group;
    root->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    root->addChild( sgA.get() );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root );
    return( viewer.run() );
}
