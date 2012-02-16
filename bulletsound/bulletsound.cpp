// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.


#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>

#include <osgbDynamics/RigidBody.h>
#include <osgbDynamics/MotionState.h>
#include <osgbDynamics/GroundPlane.h>

#include <osgbCollision/CollisionShapes.h>
#include <osgbCollision/Utils.h>

#include <osgAudio/SoundManager.h>
#include <osgAudio/SoundRoot.h>
#include <osgAudio/SoundUpdateCB.h>

#include <osgwTools/FindNamedNode.h>
#include <osgwTools/InsertRemove.h>
#include <osgwTools/Shapes.h>
#include <osgwTools/AbsoluteModelTransform.h>

#include <btBulletDynamicsCommon.h>

#include "SoundUtilities.h"
#include "Material.h"

#include <osg/io_utils>
#include <osg/MatrixTransform>

#include <iostream>


class SoundManipulator : public osgGA::GUIEventHandler
{
public:
    SoundManipulator( osg::Node* toggleNode )
      : _toggleNode( toggleNode )
    {
        SoundUtilities::instance()->setAmbient( std::string( "cricket1.wav" ) );
    }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
    {
        switch( ea.getEventType() )
        {
            case osgGA::GUIEventAdapter::KEYUP:
            {
                if( (ea.getKey()=='s') || (ea.getKey()=='S') )
                {
                    // Toggle sound the toggleNode.
                    if( !_toggleNode.valid() )
                        return( false );

                    osgAudio::SoundUpdateCB* ucb = dynamic_cast<
                        osgAudio::SoundUpdateCB* >( _toggleNode->getUpdateCallback() );
                    osgAudio::SoundState* ss( ucb->getSoundState() );
                    ss->setPlay( !( ss->getPlay() ) );

                    return true;
                }
                else if( (ea.getKey()=='a') || (ea.getKey()=='A') )
                {
                    // Toggle ambient sound
                    SoundUtilities::instance()->setAmbient( 
                        !( SoundUtilities::instance()->getAmbient() ) );
                    return true;
                }

                return false;
            }

            default:
            break;
        }
        return false;
    }

protected:
    osg::ref_ptr< osg::Node > _toggleNode;
};


// Collision flags, mainly so that the door doesn't collide with the doorframe.
enum CollisionTypes {
    COL_DOOR = 0x1 << 0,
    COL_DOORFRAME = 0x1 << 1,
    COL_DEFAULT = 0x1 << 2,
};
unsigned int doorCollidesWith( COL_DEFAULT );
unsigned int doorFrameCollidesWith( COL_DEFAULT );
unsigned int defaultCollidesWith( COL_DOOR | COL_DOORFRAME | COL_DEFAULT );


typedef std::set< btCollisionObject* > BulletObjList;
BulletObjList g_movingList;

void triggerSounds( const btDynamicsWorld* world, btScalar timeStep )
{
    // Loop over all collision ovjects and find the ones that are
    // moving. Need this for door creak.
    const btCollisionObjectArray& colObjs( world->getCollisionObjectArray() );
    int idx( world->getNumCollisionObjects() );
    while( idx-- )
    {
        btCollisionObject* co( colObjs[ idx ] );
        btVector3 v( co->getInterpolationLinearVelocity() );
        v[0] = osg::absolute< float >( v[0] );
        v[1] = osg::absolute< float >( v[1] );
        v[2] = osg::absolute< float >( v[2] );

        BulletObjList::const_iterator it( g_movingList.find( co ) );
        if( ( v[0] > .9f ) || ( v[1] > .9f ) || ( v[2] > .9f ) )
        {
            // It's moving.
            if( it == g_movingList.end() )
            {
                g_movingList.insert( co );
                // We didn't already play a sound, so play one now.
                Material* mc = ( Material* )( co->getUserPointer() );
                if( mc != NULL )
                    SoundUtilities::instance()->move( mc->_mat,
                        osgbCollision::asOsgVec3( co->getWorldTransform().getOrigin() ) );
            }
        }
        else
        {
            // it's not moving
            if( it != g_movingList.end() )
                g_movingList.erase( it );
        }
    }


    // Loop over all collision points and find impacts.
    const btCollisionDispatcher* dispatch( static_cast< const btCollisionDispatcher* >( world->getDispatcher() ) );
    const int numManifolds( dispatch->getNumManifolds() );

	for( idx=0; idx < numManifolds; idx++ )
	{
		const btPersistentManifold* contactManifold( dispatch->getManifoldByIndexInternal( idx ) );
		const btCollisionObject* obA( static_cast< const btCollisionObject* >( contactManifold->getBody0() ) );
		const btCollisionObject* obB( static_cast< const btCollisionObject* >( contactManifold->getBody1() ) );

        bool collide( false ), slide( false );
        osg::Vec3 location;

		const int numContacts( contactManifold->getNumContacts() );
        int jdx;
		for( jdx=0; jdx < numContacts; jdx++ )
		{
			const btManifoldPoint& pt( contactManifold->getContactPoint( jdx) );
            location = osgbCollision::asOsgVec3( pt.getPositionWorldOnA() );
            if( pt.m_lifeTime < 3 )
            {
                if( pt.m_appliedImpulse > 5. ) // Kind of a hack.
                    collide = true;
            }
            else
            {
                osg::Vec3 vA( osgbCollision::asOsgVec3( obA->getInterpolationLinearVelocity() ) );
                osg::Vec3 vB( osgbCollision::asOsgVec3( obB->getInterpolationLinearVelocity() ) );
                if( (vA-vB).length2() > .1 )
                    slide = true;
            }
		}
        if( collide || slide )
        {
            Material* mcA = ( Material* )( obA->getUserPointer() );
            Material* mcB = ( Material* )( obB->getUserPointer() );
            if( ( mcA != NULL ) && ( mcB != NULL ) )
            {
                if( collide )
                    SoundUtilities::instance()->collide( mcA->_mat, mcB->_mat, location );
                else
                    SoundUtilities::instance()->slide( mcA->_mat, mcB->_mat, location );
            }
        }
	}
}


btDiscreteDynamicsWorld*
initPhysics()
{
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher( collisionConfiguration );
    btConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

    btVector3 worldAabbMin( -10000, -10000, -10000 );
    btVector3 worldAabbMax( 10000, 10000, 10000 );
    btAxisSweep3* as3( new btAxisSweep3( worldAabbMin, worldAabbMax, 1000 ) );
    btBroadphaseInterface* inter = as3;

    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, inter, solver, collisionConfiguration );
    dynamicsWorld->setGravity( btVector3( 0, 0, -10 ) );

    dynamicsWorld->setInternalTickCallback( (btInternalTickCallback) triggerSounds);

    return( dynamicsWorld );
}

void
cleanupPhysics( btDynamicsWorld* bw )
{
    btCollisionObjectArray objs( bw->getCollisionObjectArray() );
    int idx;
    for( idx=0; idx<bw->getNumCollisionObjects(); idx++ )
    {
        Material* mc = ( Material* )( objs[ idx ]->getUserPointer() );
        if( mc != NULL )
            delete mc;
    }
}



void
enablePhysics( osg::Node* root, const std::string& nodeName, btDiscreteDynamicsWorld* bw )
{
    osgwTools::FindNamedNode fnn( nodeName );
    root->accept( fnn );
    if( fnn._napl.empty() )
    {
        osg::notify( osg::WARN ) << "Can't find node \"" << nodeName << "\"" << std::endl;
        return;
    }
    osg::Node* node = fnn._napl[ 0 ].first;
    osg::BoundingSphere bs( node->getBound() );
    osg::Group* parent = node->getParent( 0 );
    osg::NodePath np = fnn._napl[ 0 ].second;
    const osg::Matrix parentTrans = osg::computeLocalToWorld( np ); // Note that this might contain a scale.


    // Copy the subgraph for use with OSGToCollada.
    osg::Group* asGrp = node->asGroup();
    osg::ref_ptr< osg::Group > copyGrp = new osg::Group( *asGrp, osg::CopyOp::DEEP_COPY_ALL );

    osgbBullet::OSGToCollada converter;
    converter.setSceneGraph( copyGrp.get() );
    converter.setShapeType( BOX_SHAPE_PROXYTYPE );
    converter.setMass( 1. );
    converter.setOverall( true );

    converter.convert();

    osg::ref_ptr< osgwTools::AbsoluteModelTransform > model( new osgwTools::AbsoluteModelTransform );
    model->setDataVariance( osg::Object::DYNAMIC );
    osgwTools::insertAbove( node, model.get() );

    btRigidBody* rb = converter.getRigidBody();
    osgbDynamics::MotionState* motion = new osgbDynamics::MotionState;
    rb->setUserPointer( new Material( Material::SILLY_PUTTY ) );

    motion->setTransform( model.get() );
    if( bs.center() != osg::Vec3( 0., 0., 0. ) )
        // If we don't have an explicit COM, and OSGToCollada auto compute was enabled (default),
        // then we need to explicitly set the COM here to be the bounding sphere center.
        motion->setCenterOfMass( bs.center() );
    // The parent transform (the local to world matrix extracted from the NodePath) might contain
    // a scale. However, the setParentTransform function orthonormalizes the matrix, which
    // eliminates any scale. So, even though Bullet doesn't support scaling, you can still pass in
    // a parent transform with a scale, and MotionState will just ignore it. (You must pass in any
    // parent scale transform using setScale.)
    motion->setParentTransform( parentTrans );
    rb->setMotionState( motion );
    rb->setAngularVelocity( btVector3( 3., 5., 0. ) );
    bw->addRigidBody( rb, COL_DEFAULT, defaultCollidesWith );
}


class InteractionManipulator : public osgGA::GUIEventHandler
{
public:
    InteractionManipulator( btDiscreteDynamicsWorld* world, osg::Group* sg )
      : _world( world ),
        _sg( sg )
    {}

    void setInitialTransform( btRigidBody* rb, osg::Matrix m )
    {
        _posMap[ rb ] = m;
    }

    void updateView( osg::Camera* camera )
    {
        osg::Vec3 center, up;
        camera->getViewMatrixAsLookAt( _viewPos, center, up );
        _viewDir = center - _viewPos;
    }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
    {
        switch( ea.getEventType() )
        {
            case osgGA::GUIEventAdapter::KEYUP:
            {
                if (ea.getKey()==osgGA::GUIEventAdapter::KEY_BackSpace)
                {
                    reset();
                    return true;
                }
                if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Return)
                {
                    fire();
                    return true;
                }

                return false;
            }

            default:
            break;
        }
        return false;
    }

protected:
    btDiscreteDynamicsWorld* _world;
    osg::ref_ptr< osg::Group > _sg;

    osg::Vec3 _viewPos, _viewDir;

    typedef std::map< btRigidBody*, osg::Matrix > PosMap;
    PosMap _posMap;

    typedef std::list< osg::ref_ptr< osg::Node > > NodeList;
    NodeList _nodeList;

    void reset()
    {
        PosMap::iterator it;
        for( it=_posMap.begin(); it!=_posMap.end(); it++ )
        {
            btRigidBody* rb = it->first;
            btTransform t = osgbCollision::asBtTransform( it->second );
            rb->setWorldTransform( t );
        }
    }

    void fire()
    {
        osg::Geode* geode = new osg::Geode();
        geode->addDrawable( osgwTools::makeGeodesicSphere( .5 ) );
        osg::ref_ptr< osgwTools::AbsoluteModelTransform > amt = new osgwTools::AbsoluteModelTransform;
        amt->addChild( geode );
        _sg->addChild( amt.get() );

        btSphereShape* collision = new btSphereShape( .5 );

        osgbDynamics::MotionState* motion = new osgbDynamics::MotionState;
        motion->setTransform( amt.get() );

        motion->setParentTransform( osg::Matrix::translate( _viewPos ) );

        btScalar mass( 1. );
        btVector3 inertia( btVector3( 0., 0., 0. ) );//osgbBullet::asBtVector3( _viewDir ) );
        collision->calculateLocalInertia( mass, inertia );
        btRigidBody::btRigidBodyConstructionInfo rbinfo( mass, motion, collision, inertia );
        btRigidBody* body = new btRigidBody( rbinfo );
        body->setLinearVelocity( osgbCollision::asBtVector3( _viewDir * 50. ) );
        body->setUserPointer( new Material( Material::FLUBBER ) );
        _world->addRigidBody( body, COL_DEFAULT, defaultCollidesWith );

        SoundUtilities::instance()->playSound( _viewPos, "cannon_x.wav", 1.5f );
    }
};


btRigidBody* doorFrame;
osg::BoundingSphere doorbb;

osg::Transform*
makeDoorFrame( btDiscreteDynamicsWorld* bw, InteractionManipulator* im )
{
    // Create the door frame scene graph, rooted at an AMT.
    osgwTools::AbsoluteModelTransform* amt = new osgwTools::AbsoluteModelTransform;

    osg::Node* node = osgDB::readNodeFile( "USMC23_1019.ASM.ive" );
    amt->addChild( node );


    /*
    // Create matrix transform to simulate an accumulated transformation in the hierarchy.
    // Create a NodePath from it.
    // In a real app, NodePath would come from visiting the parents.
    osg::Matrix m( osg::Matrix::rotate( osg::PI, osg::Vec3( 1., 0., 0. ) ) *
            osg::Matrix::translate( osg::Vec3( 0., 0., 7.1 ) ) );
    osg::MatrixTransform* mt = new osg::MatrixTransform( m );
    osg::NodePath np;
    np.push_back( mt );
    */

    // Deep copy for conversion to collision shape
    osg::Group* asGrp = node->asGroup();
    osg::ref_ptr< osg::Group > copyGrp = new osg::Group( *asGrp, osg::CopyOp::DEEP_COPY_ALL );

    osgbBullet::OSGToCollada converter;
    converter.setSceneGraph( copyGrp.get() );
    converter.setShapeType( BOX_SHAPE_PROXYTYPE );
    converter.setMass( 0. ); // static
    converter.setOverall( true );

    converter.convert();

    btRigidBody* rb = converter.getRigidBody();
    osgbDynamics::MotionState* motion = new osgbDynamics::MotionState;
    motion->setTransform( amt );
    rb->setMotionState( motion );

    bw->addRigidBody( rb, COL_DOORFRAME, doorFrameCollidesWith );
    rb->setActivationState( DISABLE_DEACTIVATION );

    // Save RB in global, and also record its initial position in the InteractionManipulator (for reset)
    doorFrame=rb;

    rb->setUserPointer( new Material( Material::WOOD_DOOR ) );

    return( amt );
}

btRigidBody* door;
osg::Transform*
makeDoor( btDiscreteDynamicsWorld* bw, InteractionManipulator* im )
{
    // Create the door scene graph, rooted at an AMT.
    osgwTools::AbsoluteModelTransform* amt = new osgwTools::AbsoluteModelTransform;
    amt->setDataVariance( osg::Object::DYNAMIC );

    osg::Node* node = osgDB::readNodeFile( "USMC23_1020.ASM.ive" );
    amt->addChild( node );

    // We'll use this later to position the debug axes.
    doorbb = node->getBound();


    // Create matrix transform to simulate an accumulated transformation in the hierarchy.
    // Create a NodePath from it.
    // In a real app, NodePath would come from visiting the parents.
    osg::Matrix m( osg::Matrix::rotate( osg::PI_2, osg::Vec3( 0., 1., 0. ) ) * 
        osg::Matrix::translate( osg::Vec3( -.26, -3.14, .2 ) ) );
    /*
    osg::MatrixTransform* mt = new osg::MatrixTransform( m );
    osg::NodePath np;
    np.push_back( mt );
    */

    // Deep copy for conversion to collision shape
    osg::Group* asGrp = node->asGroup();
    osg::ref_ptr< osg::Group > copyGrp = new osg::Group( *asGrp, osg::CopyOp::DEEP_COPY_ALL );

    osgbBullet::OSGToCollada converter;
    converter.setSceneGraph( copyGrp.get() );
    converter.setShapeType( BOX_SHAPE_PROXYTYPE );
    converter.setMass( 1. );
    converter.setOverall( true );

    converter.convert();

    btRigidBody* rb = converter.getRigidBody();
    osgbDynamics::MotionState* motion = new osgbDynamics::MotionState;
    motion->setTransform( amt );
    rb->setMotionState( motion );

    bw->addRigidBody( rb, COL_DOOR, doorCollidesWith );
    rb->setActivationState( DISABLE_DEACTIVATION );

    // Save RB in global, and also record its initial position in the InteractionManipulator (for reset)
    door=rb;
    im->setInitialTransform( rb, m );

    rb->setUserPointer( new Material( Material::WOOD_DOOR ) );

    return( amt );
}

int main( int argc,
          char * argv[] )
{
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 600 );
    osgGA::TrackballManipulator * tb = new osgGA::TrackballManipulator();
    tb->setHomePosition( osg::Vec3( 10, -45, 10 ),
                        osg::Vec3( 0, 0, 2 ),
                        osg::Vec3( 0, 0, 1 ) );
    viewer.setCameraManipulator( tb );

    int num_hw_soundsources = 10;
    osgAudio::SoundManager::instance()->init( num_hw_soundsources );
    osgAudio::SoundManager::instance()->getEnvironment()->setDistanceModel( osgAudio::InverseDistance );
    osgAudio::SoundManager::instance()->getEnvironment()->setDopplerFactor( 1 );

    osg::ref_ptr< osg::Group > root = new osg::Group();
    osg::ref_ptr< osgAudio::SoundRoot > soundRoot( new osgAudio::SoundRoot );
    soundRoot->setCamera( viewer.getCamera() );
    root->addChild( soundRoot.get() );

    btDiscreteDynamicsWorld* bw = initPhysics();

    InteractionManipulator* im = new InteractionManipulator( bw, root.get() );
    viewer.addEventHandler( im );

    btRigidBody* groundRB;
    root->addChild( osgbDynamics::generateGroundPlane( osg::Vec4( 0.f, 0.f, 1.f, -1.f ), bw, &groundRB ) );
    groundRB->setUserPointer( new Material( Material::CEMENT ) );

    osg::MatrixTransform* mt( new osg::MatrixTransform( osg::Matrix::translate( 0.5, 0., 14. ) ) );
    root->addChild( mt );

    osg::Node* block = osgDB::readNodeFile( "C:/TestData/usmc34/osg/00top.ive" );
    block->setName( "block" );
    mt->addChild( block );
    enablePhysics( root.get(), "block", bw );

    SoundUtilities::instance()->addSound( block, "engine.wav", .35 );

    viewer.addEventHandler( new SoundManipulator( block ) );


#if( BT_BULLET_VERSION < 276 )
    // TBD currently uses loadDae which is broke with Bullet 2.76.

    // Add door
    osg::Transform* doorRoot = makeDoor( bw, im );
    root->addChild( doorRoot );

    // Add doorframe
    osg::Transform* doorFrameRoot = makeDoorFrame( bw, im );
    root->addChild( doorFrameRoot );

    // create hinge constraint
    {
        // creating a hinge constraint and adding to world
        osg::Vec3 hingePivotPoint( 0.f, -1.5f, -.12f );
        const btVector3 btPivot( hingePivotPoint.x(), hingePivotPoint.y(), hingePivotPoint.z() ); 
        btVector3 btAxisA( 1., 0., 0. ); // rotation about the x axis
        btHingeConstraint* hinge = new btHingeConstraint( *door, btPivot, btAxisA );
        hinge->setLimit( -2.8f, 0.f );
        bw->addConstraint(hinge, true);
    }
#endif


    viewer.setSceneData( root.get() );
    viewer.realize();

    double currSimTime;
    double prevSimTime = viewer.getFrameStamp()->getSimulationTime();

    while( !viewer.done() )
    {
        currSimTime = viewer.getFrameStamp()->getSimulationTime();
        bw->stepSimulation( currSimTime - prevSimTime );
        prevSimTime = currSimTime;
        viewer.frame();

        im->updateView( viewer.getCamera() );
    }

    cleanupPhysics( bw );

    SoundUtilities::instance()->shutdown( root.get() );

    osgAudio::SoundManager::instance()->shutdown();

    return( 0 );
}

