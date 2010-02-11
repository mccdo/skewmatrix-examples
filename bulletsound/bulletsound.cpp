// Copyright (c) 2009 Skew Matrix Software LLC. All rights reserved.


#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osg/ShapeDrawable>

#include <osgbBullet/OSGToCollada.h>
#include <osgbBullet/MotionState.h>
#include <osgbBullet/CollisionShapes.h>
#include <osgbBullet/Utils.h>

#include <osgAudio/SoundManager.h>
#include <osgAudio/SoundRoot.h>
#include <osgAudio/SoundNode.h>

#include <osgwTools/FindNamedNode.h>
#include <osgwTools/InsertRemove.h>

#include <btBulletDynamicsCommon.h>

#include <osg/io_utils>
#include <iostream>




#ifdef WIN32
   // Make noise, on Windows at least, until we have osgAudio.
#  define USE_WIN32_BEEP 1
#endif

void triggerSounds( const btDynamicsWorld* world, btScalar timeStep )
{
    const btCollisionDispatcher* dispatch( static_cast< const btCollisionDispatcher* >( world->getDispatcher() ) );
    const int numManifolds( dispatch->getNumManifolds() );

    int idx;
	for( idx=0; idx < numManifolds; idx++ )
	{
		const btPersistentManifold* contactManifold( dispatch->getManifoldByIndexInternal( idx ) );
		const btCollisionObject* obA( static_cast< const btCollisionObject* >( contactManifold->getBody0() ) );
		const btCollisionObject* obB( static_cast< const btCollisionObject* >( contactManifold->getBody1() ) );
	
		const int numContacts( contactManifold->getNumContacts() );
        int jdx;
		for( jdx=0; jdx < numContacts; jdx++ )
		{
			const btManifoldPoint& pt( contactManifold->getContactPoint( jdx) );
            if( pt.m_lifeTime < 3 )
            {
                if( pt.m_appliedImpulse > 10. ) // Kind of a hack.
                {
#ifdef USE_WIN32_BEEP
                    Beep( 800, 40 );
#else
                    osg::notify( osg::ALWAYS ) << "  osgAudio: Initial contact placeholder" << std::endl;
#endif
                }
            }
            else
            {
                osg::Vec3 vA( osgbBullet::asOsgVec3( obA->getInterpolationLinearVelocity() ) );
                osg::Vec3 vB( osgbBullet::asOsgVec3( obB->getInterpolationLinearVelocity() ) );
                //osg::notify( osg::ALWAYS ) <<
                //    "    linvelA: " << vA <<
                //    ", linvelB: " << vB << std::endl;
                if( (vA-vB).length2() > .1 )
                {
#ifdef USE_WIN32_BEEP
                    Beep( 400, 30 );
#else
                    osg::notify( osg::ALWAYS ) << "  osgAudio: Scraping placeholder" << std::endl;
#endif
                }
            }
/*
            if (pt.getDistance()<0.f)
			{
				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				const btVector3& normalOnB = pt.m_normalWorldOnB;
			}
            */
		}
	}
}


btDynamicsWorld* initPhysics()
{
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher( collisionConfiguration );
    btConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

    btVector3 worldAabbMin( -10000, -10000, -10000 );
    btVector3 worldAabbMax( 10000, 10000, 10000 );
    btAxisSweep3* as3( new btAxisSweep3( worldAabbMin, worldAabbMax, 1000 ) );
    btBroadphaseInterface* inter = as3;

    btDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, inter, solver, collisionConfiguration );
    dynamicsWorld->setGravity( btVector3( 0, 0, -10 ) );

    dynamicsWorld->setInternalTickCallback( (btInternalTickCallback) triggerSounds);

    return( dynamicsWorld );
}



void
enablePhysics( osg::Node* root, const std::string& nodeName, btDynamicsWorld* bw )
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
    osgbBullet::MotionState* motion = new osgbBullet::MotionState;

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
    bw->addRigidBody( rb );
}


class InteractionManipulator : public osgGA::GUIEventHandler
{
public:
    InteractionManipulator( btDynamicsWorld* world, osg::Group* sg )
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
    btDynamicsWorld* _world;
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
            btTransform t = osgbBullet::asBtTransform( it->second );
            rb->setWorldTransform( t );
        }
    }

    void fire()
    {
        osg::Sphere* sp = new osg::Sphere( osg::Vec3( 0., 0., 0. ), .5 );
        osg::ShapeDrawable* shape = new osg::ShapeDrawable( sp );
        osg::Geode* geode = new osg::Geode();
        geode->addDrawable( shape );
        osg::ref_ptr< osgwTools::AbsoluteModelTransform > amt = new osgwTools::AbsoluteModelTransform;
        amt->addChild( geode );
        _sg->addChild( amt.get() );

        btSphereShape* collision = new btSphereShape( .5 );

        osgbBullet::MotionState* motion = new osgbBullet::MotionState;
        motion->setTransform( amt.get() );

        motion->setParentTransform( osg::Matrix::translate( _viewPos ) );

        btScalar mass( 1. );
        btVector3 inertia( btVector3( 0., 0., 0. ) );//osgbBullet::asBtVector3( _viewDir ) );
        collision->calculateLocalInertia( mass, inertia );
        btRigidBody::btRigidBodyConstructionInfo rbinfo( mass, motion, collision, inertia );
        btRigidBody* body = new btRigidBody( rbinfo );
        body->setLinearVelocity( osgbBullet::asBtVector3( _viewDir * 50. ) );
        _world->addRigidBody( body );
    }
};


void
addSound( osg::Group* grp, const std::string& fileName )
{
    const bool addToCache( true );
    osg::ref_ptr< osgAudio::Sample > sample( osgAudio::SoundManager::instance()->getSample( fileName, addToCache ) );
    osg::notify( osg::WARN ) << "Loading sample: " << fileName << std::endl;

    // Create a new soundstate, give it the name of the file we loaded.
    osg::ref_ptr< osgAudio::SoundState > soundState( new osgAudio::SoundState( fileName ) );
    soundState->setSample( sample.get() );
    soundState->setGain( 1.0f );
    soundState->setReferenceDistance( 60 );
    soundState->setRolloffFactor( 3 );
    soundState->setPlay( true );
    soundState->setLooping( true );

    // Allocate a hardware soundsource to this soundstate (priority 10)
    soundState->allocateSource( 10, false );

    // Add the soundstate to the sound manager, so we can find it later on if we want to
    osgAudio::SoundManager::instance()->addSoundState( soundState.get() );

    soundState->apply();

    osg::ref_ptr< osgAudio::SoundNode > soundNode( new osgAudio::SoundNode );
    soundNode->setSoundState( soundState );
    grp->addChild( soundNode.get() );
}


int main( int argc,
          char * argv[] )
{
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 600 );
    osgGA::TrackballManipulator * tb = new osgGA::TrackballManipulator();
    tb->setHomePosition( osg::Vec3( 10, -55, 13 ),
                        osg::Vec3( 0, 0, -4 ),
                        osg::Vec3( 0, 0, 1 ) );
    viewer.setCameraManipulator( tb );

    int num_hw_soundsources = 10;
    osgAudio::SoundManager::instance()->init( num_hw_soundsources );
    osgAudio::SoundManager::instance()->getEnvironment()->setDistanceModel( osgAudio::InverseDistance );
    osgAudio::SoundManager::instance()->getEnvironment()->setDopplerFactor( 1 );

    osg::ref_ptr< osg::Group > root = new osg::Group();
    osg::ref_ptr< osgAudio::SoundRoot > soundRoot( new osgAudio::SoundRoot );
    root->addChild( soundRoot.get() );

    btDynamicsWorld* bw = initPhysics();

    InteractionManipulator* im = new InteractionManipulator( bw, root.get() );
    viewer.addEventHandler( im );

    root->addChild( osgbBullet::generateGroundPlane( osg::Vec4( 0.f, 0.f, 1.f, -10.f ), bw ) );

    osg::MatrixTransform* mt( new osg::MatrixTransform( osg::Matrix::translate( 0., 0., 10. ) ) );
    root->addChild( mt );

    osg::Node* block = osgDB::readNodeFile( "block.osg" );
    block->setName( "block" );
    mt->addChild( block );
    enablePhysics( root.get(), "block", bw );

    addSound( block->getParent(0), "bee.wav" );


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

    return( 0 );
}

