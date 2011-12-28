// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include "character.h"

#include <osgDB/ReadFile>
#include <osg/ComputeBoundsVisitor>
#include <osg/BoundingBox>
#include <osg/MatrixTransform>

#include <osgwTools/Shapes.h>
#include <osgwTools/Transform.h>

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <osgbCollision/Utils.h>

#include <osg/io_utils>


Character::Character( btDynamicsWorld* bw )
  : _model( NULL ),
    _capsule( NULL ),
    _capsuleHeight( 6. ),
    _capsuleRadius( 1.25 ),
    _bw( bw ),
    _btChar( NULL ),
    _btGhost( NULL ),
    _capsuleShape( NULL )
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

    _lastPosition.set( 0., 0., 5. );
    _btGhost->setWorldTransform( osgbCollision::asBtTransform(
        osg::Matrix::translate( _lastPosition ) ) );

    _bw->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs( _btGhost->getBroadphaseHandle(), _bw->getDispatcher() );

    return( _root.get() );
}

void Character::setModelVisible( bool visible )
{
    _model->setNodeMask( visible ? 0xffffffff : 0x0 );
}
bool Character::getModelVisible() const
{
    return( ( _model->getNodeMask() & 0xffffffff ) != 0 );
}

void Character::setCapsuleVisible( bool visible )
{
    _capsule->setNodeMask( visible ? 0xffffffff : 0x0 );
}
bool Character::getCapsuleVisible() const
{
    return( ( _capsule->getNodeMask() & 0xffffffff ) != 0 );
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

void Character::setPhysicsWorldTransform( const osg::Matrix& m )
{
    if( _btGhost == NULL )
        return;

    // Account for MxCore orientation.
    osg::Matrix orient = osg::Matrix::rotate( -osg::PI_2, 1., 0., 0. );
    osg::Matrix worldMatrix = orient * m;

    osg::Vec3 deltaStep = worldMatrix.getTrans() - _lastPosition;
    _lastPosition = worldMatrix.getTrans();
    if( _btChar != NULL )
    {
        _btChar->setWalkDirection( osgbCollision::asBtVector3( deltaStep ) );
    }
}

void Character::setOSGMatrix( const osg::Matrix& m )
{
    // Account for MxCore orientation.
    osg::Matrix orient = osg::Matrix::translate( 0., 0., _capsuleHeight * -.5 ) *
        osg::Matrix::rotate( -osg::PI_2, 1., 0., 0. );
    osg::Matrix worldMatrix = orient * m;
    _root->setMatrix( worldMatrix );
}

osg::Vec3 Character::getPosition() const
{
    if( _btGhost == NULL )
        return( osg::Vec3() );

    return( osgbCollision::asOsgVec3(
        _btGhost->getWorldTransform().getOrigin() ) );
}


void Character::generateCapsule()
{
    osg::Geode* geode = new osg::Geode;
    osg::Geometry* geom = osgwTools::makeWireCapsule( _capsuleHeight, _capsuleRadius );
    geode->addDrawable( geom );
    _capsule = geode;

    if( _capsuleShape != NULL )
        delete( _capsuleShape );
    _capsuleShape = new btCapsuleShapeZ( _capsuleRadius,
        _capsuleHeight - ( _capsuleRadius * 2. ) );

    if( _btGhost != NULL )
    {
        _bw->removeCollisionObject( _btGhost );
        delete _btGhost;
    }
    _btGhost = new btPairCachingGhostObject();
	_btGhost->setCollisionShape( _capsuleShape );
	_btGhost->setCollisionFlags( btCollisionObject::CF_CHARACTER_OBJECT );

    if( _btChar != NULL )
    {
        _bw->removeAction( _btChar );
        delete _btChar;
    }
	_btChar = new btKinematicCharacterController( _btGhost, _capsuleShape, 1.0, 2 );

    _bw->addCollisionObject( _btGhost, btBroadphaseProxy::CharacterFilter,
        btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter );
	_bw->addAction( _btChar );
}
