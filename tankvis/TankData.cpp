// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include "TankData.h"
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Texture2D>
#include <osg/ComputeBoundsVisitor>

#include <osg/io_utils>


TankData::TankData( osg::Node* node )
  : _node( node ),
    _up( osg::Vec3( 0., 0., 1. ) ),
    _percent( 1.f ),
    _colorMethod( COLOR_OFF ),
    _explicitColor( osg::Vec4( 0., 0., 0., 1. ) ),
    _fluidType( std::string( "" ) ),
    _fluidMap( NULL )
{
    if( _node.valid() )
        updateAll();
}
TankData::~TankData()
{
}

void TankData::updateAll()
{
    setUp( _up );
    setPercentOfCapacity( _percent );

    switch( _colorMethod )
    {
    case COLOR_OFF:
        updateColor();
    case COLOR_EXPLICIT:
        setExplicitColor( _explicitColor );
    case COLOR_FLUID_TYPE:
        setFluidType( _fluidType );
    }


    osg::StateSet* stateSet = _node->getOrCreateStateSet();

    osg::Program* program = new osg::Program;
    std::string fileName = "tank.vs";
    osg::Shader* shader = osg::Shader::readShaderFile( osg::Shader::VERTEX,
        osgDB::findDataFile( fileName ) );
    if( shader == NULL )
        osg::notify( osg::WARN ) << "Can't load " << fileName << std::endl;
    shader->setName( fileName );
    program->addShader( shader );
    fileName = "tank.fs";
    shader = osg::Shader::readShaderFile( osg::Shader::FRAGMENT,
        osgDB::findDataFile( fileName ) );
    if( shader == NULL )
        osg::notify( osg::WARN ) << "Can't load " << fileName << std::endl;
    shader->setName( fileName );
    program->addShader( shader );
    stateSet->setAttribute( program );

    osg::Uniform* u = stateSet->getOrCreateUniform( "base", osg::Uniform::SAMPLER_2D );
    u->set( 0 );
    u = stateSet->getOrCreateUniform( "lightmap", osg::Uniform::SAMPLER_2D );
    u->set( 1 );

    fileName = std::string( "water.png" );
    osg::Image* image = osgDB::readImageFile( fileName );
    if( image == NULL )
        osg::notify( osg::WARN ) << "Can't load " << fileName << std::endl;
    osg::Texture2D* tex = new osg::Texture2D( image );
    tex->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
    tex->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    tex->setUseHardwareMipMapGeneration( true );
    stateSet->setTextureAttribute( 15, tex );
    u = stateSet->getOrCreateUniform( "watertex", osg::Uniform::SAMPLER_2D );
    u->set( 15 );

    osg::ComputeBoundsVisitor cbv;
    _node->accept( cbv );
    u = stateSet->getOrCreateUniform( "minExtent", osg::Uniform::FLOAT_VEC3 );
    u->set( cbv.getBoundingBox()._min );
    u = stateSet->getOrCreateUniform( "maxExtent", osg::Uniform::FLOAT_VEC3 );
    u->set( cbv.getBoundingBox()._max );
}
void TankData::updateColor()
{
    osg::StateSet* stateSet = _node->getOrCreateStateSet();
    osg::Uniform* u = stateSet->getOrCreateUniform( "fluidColor", osg::Uniform::FLOAT_VEC4 );

    switch( _colorMethod )
    {
    case COLOR_OFF:
        u->set( osg::Vec4f( 0., 0., 0., 1. ) );
    case COLOR_EXPLICIT:
        u->set( _explicitColor );
    case COLOR_FLUID_TYPE:
        if( _fluidMap != NULL )
            u->set( (* _fluidMap )[ _fluidType ] );
    }
}

void TankData::setNode( osg::Node* node )
{
    _node = node;

    if( _node.valid() )
        updateAll();
}
osg::Node* TankData::getNode()
{
    return( _node.get() );
}
const osg::Node* TankData::getNode() const
{
    return( _node.get() );
}

void TankData::setUp( const osg::Vec3f& up )
{
    _up = up;

    if( _node.valid() )
    {
        osg::StateSet* stateSet = _node->getOrCreateStateSet();
        osg::Uniform* u = stateSet->getOrCreateUniform( "up", osg::Uniform::FLOAT_VEC3 );
        osg::Vec3f localUp = _up;
        localUp.normalize();
        u->set( localUp );
    }
}
const osg::Vec3& TankData::getUp() const
{
    return( _up );
}

void TankData::setPercentOfCapacity( float percent )
{
    _percent = percent;

    if( _node.valid() )
    {
        osg::StateSet* stateSet = _node->getOrCreateStateSet();
        osg::Uniform* u = stateSet->getOrCreateUniform( "percent", osg::Uniform::FLOAT );
        u->set( _percent );
    }
}
float TankData::setPercentOfCapacity() const
{
    return( _percent );
}

void TankData::setColorMethod( const ColorMethod& colorMethod )
{
    _colorMethod = colorMethod;
    updateColor();
}
const TankData::ColorMethod& TankData::getColorMethod() const
{
    return( _colorMethod );
}

void TankData::setExplicitColor( const osg::Vec4& explicitColor )
{
    _explicitColor = explicitColor;
    updateColor();
}
const osg::Vec4& TankData::getExplicitColor() const
{
    return( _explicitColor );
}

void TankData::setFluidType( const std::string& fluidType )
{
    _fluidType = fluidType;
    updateColor();
}
const std::string& TankData::getFluidType() const
{
    return( _fluidType );
}

void TankData::setFluidMap( FluidTypeColorMap* fluidMap )
{
    _fluidMap = fluidMap;
    updateColor();
}
const TankData::FluidTypeColorMap* TankData::getFluidMap() const
{
    return( _fluidMap );
}
