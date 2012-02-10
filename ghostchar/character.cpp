// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include "character.h"

//#define USE_BULLET_KINEMATIC_CHARACTER 1
#ifdef USE_BULLET_KINEMATIC_CHARACTER
#  include <BulletDynamics/Character/btKinematicCharacterController.h>
#else
#  include "LocalKinematicCharacterController.h"
#endif

#include <osgDB/ReadFile>
#include <osg/ComputeBoundsVisitor>
#include <osg/BoundingBox>
#include <osg/MatrixTransform>

#include <osgwTools/Shapes.h>
#include <osgwTools/Transform.h>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <osgbCollision/Utils.h>

#include <osg/io_utils>


Character::Character( btDynamicsWorld* bw )
  : _model( NULL ),
    _capsule( NULL ),
    _modelHeight( 6. ),
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

    _root = new osg::Group;
    _modelTransform = new osg::MatrixTransform;
    _root->addChild( _modelTransform.get() );
    _modelTransform->addChild( _model.get() );


    osg::ComputeBoundsVisitor cbv;
    _model->accept( cbv );
    const osg::BoundingBox& bb = cbv.getBoundingBox();
    _modelHeight = bb._max[2] - bb._min[2];
    _capsuleRadius = _modelHeight * 1.25 / 6.; // For 6 foot tall, this gives us 1.25 foot radius.
    _capsuleHeight = _modelHeight;

    _capsuleTransform = new osg::MatrixTransform;
    _root->addChild( _capsuleTransform.get() );
    generateCapsule();

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
    if( !( _root.valid() ) )
        return;

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
    if( _btChar != NULL )
    {
        _btChar->setWalkDirection( osgbCollision::asBtVector3( deltaStep ) );
    }
}

void Character::setOSGMatrix( const osg::Matrix& m )
{
    // Clamp capsule height minimum at 2x capsule radius.
    // Otherwise, Bullet collision shape appears to perform poorly.
    const double capsuleHeight = osg::maximum< double >( _capsuleHeight, _capsuleRadius * 2. );

    // Account for MxCore orientation.
    double z = -( ( 2. * _modelHeight - capsuleHeight ) * .5 );
    osg::Matrix orient = osg::Matrix::translate( 0., 0., z ) *
        osg::Matrix::rotate( -osg::PI_2, 1., 0., 0. );
    _modelTransform->setMatrix( orient * m );

    orient = osg::Matrix::translate( 0., 0., capsuleHeight * -.5 ) *
        osg::Matrix::rotate( -osg::PI_2, 1., 0., 0. );
    _capsuleTransform->setMatrix( orient * m );

    _lastPosition = m.getTrans();
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
    // Clamp capsule height minimum at 2x capsule radius.
    // Otherwise, Bullet collision shape appears to perform poorly.
    const double capsuleHeight = osg::maximum< double >( _capsuleHeight, _capsuleRadius * 2. );

    unsigned int nodeMask( 0xffffffff );
    if( _capsule != NULL )
    {
        // We're going to delete and recreate the capsule geometry.
        // Save the nodemask for possible restore, so that if the user
        // turns it off, it stays off after we regenerate the geometry.
        nodeMask = _capsule->getNodeMask();
        _capsuleTransform->removeChild( _capsule.get() );
    }

    osg::Geode* geode = new osg::Geode;
    osg::Geometry* geom = osgwTools::makeWireCapsule( capsuleHeight, _capsuleRadius );
    geode->addDrawable( geom );
    _capsule = geode;
    _capsuleTransform->addChild( _capsule.get() );
    if( nodeMask != _capsule->getNodeMask() )
        _capsule->setNodeMask( nodeMask );

    if( _capsuleShape != NULL )
        delete( _capsuleShape );
    _capsuleShape = new btCapsuleShapeZ( _capsuleRadius,
        capsuleHeight - ( _capsuleRadius * 2. ) );

    if( _btGhost == NULL )
    {
        _btGhost = new btPairCachingGhostObject();
	    _btGhost->setCollisionShape( _capsuleShape );
	    _btGhost->setCollisionFlags( btCollisionObject::CF_CHARACTER_OBJECT );
        _bw->addCollisionObject( _btGhost ); //, btBroadphaseProxy::CharacterFilter,
            //btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter );
    }
    else
    {
	    _btGhost->setCollisionShape( _capsuleShape );
    }

    if( _btChar != NULL )
    {
        _bw->removeAction( _btChar );
        delete _btChar;
    }
	_btChar = new btKinematicCharacterController( _btGhost, _capsuleShape, 1.0, 2 );
	_bw->addAction( _btChar );
}
