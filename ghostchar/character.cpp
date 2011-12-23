// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include "character.h"

#include <osgDB/ReadFile>
#include <osg/ComputeBoundsVisitor>
#include <osg/BoundingBox>
#include <osg/MatrixTransform>

#include <osgwTools/Shapes.h>
#include <osgwTools/Transform.h>


Character::Character()
  : _model( NULL ),
    _capsule( NULL ),
    _modelVisible( true ),
    _capsuleVisible( true ),
    _capsuleHeight( 6. ),
    _capsuleRadius( 1.25 )
{
}
Character::~Character()
{
}

osg::Group* Character::setModel( const std::string& fileName, bool transform )
{
    osg::Node* model = osgDB::readNodeFile( fileName );
    if( model == NULL )
    {
        osg::notify( osg::WARN ) << "Character::setModel(): Could not load " << fileName << std::endl;
        return( NULL );
    }
    if( transform )
    {
        // Model is +y up, +z forward. Transform it to +z up, +y forward.
        osg::Matrix m = osg::Matrix::rotate( osg::PI_2, 1., 0., 0. ) *
            osg::Matrix::rotate( osg::PI, 0., 0., 1. );
        osg::MatrixTransform* mt = new osg::MatrixTransform( m );
        mt->addChild( model );
        _model = mt;
    }
    else
        _model = model;

    _root = new osg::MatrixTransform;
    _root->addChild( _model );

    osg::ComputeBoundsVisitor cbv;
    _model->accept( cbv );
    const osg::BoundingBox& bb = cbv.getBoundingBox();
    double bbz = bb._max[2] - bb._min[2];
    _capsuleRadius = bbz * 1.25 / 6.; // For 6 foot tall, this gives us 1.25 foot radius.
    _capsuleHeight = bbz;
    generateCapsule();
    _root->addChild( _capsule );

    return( _root.get() );
}

void Character::setHeight( double height )
{
    _capsuleHeight = height;
    generateCapsule();
}
double Character::getHeight() const
{
    return( _capsuleHeight );
}

void Character::setMatrix( const osg::Matrix m )
{
    // Account for MxCore orientation.
    osg::Matrix orient = osg::Matrix::rotate( -osg::PI_2, 1., 0., 0. );
    _root->setMatrix( orient * m );
}

void Character::generateCapsule()
{
    osg::Geode* geode = new osg::Geode;
    osg::Geometry* geom = osgwTools::makeWireCapsule( _capsuleHeight, _capsuleRadius );
    geode->addDrawable( geom );
    _capsule = geode;
}
