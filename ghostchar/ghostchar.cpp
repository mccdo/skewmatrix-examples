// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/GUIEventHandler>
#include <osg/MatrixTransform>
#include <osg/Geode>

#include <osgwTools/AbsoluteModelTransform.h>
#include <osgwTools/Shapes.h>
#include <osgbDynamics/CreationRecord.h>
#include <osgbInteraction/LaunchHandler.h>
#include <osgbCollision/GLDebugDrawer.h>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#ifdef DIRECTINPUT_ENABLED
#include <osgwMx/MxGamePadDX.h>
#else
#include <osgwMx/MxEventHandler.h>
#endif

#include "world.h"
#include "character.h"

#include <osg/io_utils>



/** \cond */
class KeyHandler : public osgGA::GUIEventHandler
{
public:
    enum ViewMode {
        LOCAL,
        FOLLOW,
        GLOBAL
    };

    KeyHandler( osgwMx::MxCore* mxCore )
      : _mxCore( mxCore ),
        _viewMode( FOLLOW )
    {
        _localOffsetEC = osg::Matrix::translate( 0., -2.5, .5 ) *
            osg::Matrix::rotate( .05, 1., 0., 0. );
        _followOffsetEC = osg::Matrix::translate( 0., -7., -11. ) *
            osg::Matrix::rotate( .5, 1., 0., 0. );
        osg::Vec3 dir( -7.5, 8., -5. );
        dir.normalize();
        _globalDirWC = dir * 40.;
    }

    void setViewMatrix( osg::Camera* camera )
    {
        switch( _viewMode )
        {
        case LOCAL:
            camera->setViewMatrix( _mxCore->getInverseMatrix() * _localOffsetEC );
            break;
        case FOLLOW:
            camera->setViewMatrix( _mxCore->getInverseMatrix() * _followOffsetEC );
            break;
        case GLOBAL:
            const osg::Vec3 pos = _mxCore->getPosition();
            const osg::Vec3 eye = pos - _globalDirWC;
            camera->setViewMatrix( osg::Matrix::lookAt( eye, pos, osg::Vec3( 0., 0., 1. ) ) );
            break;
        }
    }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
    {
        bool handled( false );
        switch( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::KEYDOWN:
        {
            switch (ea.getKey())
            {
            case osgGA::GUIEventAdapter::KEY_F5:
            {
                _viewMode = LOCAL;
                handled = true;
                break;
            }
            case osgGA::GUIEventAdapter::KEY_F6:
            {
                _viewMode = FOLLOW;
                handled = true;
                break;
            }
            case osgGA::GUIEventAdapter::KEY_F7:
            {
                _viewMode = GLOBAL;
                handled = true;
                break;
            }
            case osgGA::GUIEventAdapter::KEY_F8:
            {
                // Cycle
                switch( _viewMode )
                {
                case LOCAL: _viewMode = FOLLOW; break;
                case FOLLOW: _viewMode = GLOBAL; break;
                case GLOBAL: _viewMode = LOCAL; break;
                }
                handled = true;
                break;
            }
            }
            break;
        }
        }
        return( handled );
    }

protected:
    ~KeyHandler() {};

    osg::ref_ptr< osgwMx::MxCore > _mxCore;
    ViewMode _viewMode;

    osg::Matrix _localOffsetEC;
    osg::Matrix _followOffsetEC;
    osg::Vec3 _globalDirWC;
};
/** \endcond */


btDynamicsWorld* initPhysics()
{
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher( collisionConfiguration );
    btConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

    btVector3 worldAabbMin( -1000, -1000, -1000 );
    btVector3 worldAabbMax( 1000, 1000, 1000 );
    btBroadphaseInterface* inter = new btAxisSweep3( worldAabbMin, worldAabbMax );

    btDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, inter, solver, collisionConfiguration );
    dynamicsWorld->getDispatchInfo().m_allowedCcdPenetration = 0.0001f;

    // Support for ghost objects
    inter->getOverlappingPairCache()->setInternalGhostPairCallback(
        new btGhostPairCallback() );

    // Set gravity in ft/sec^2: accel due to gravity is ~9.8 m/s^2 * 3.28 ft/m = 32.14
    dynamicsWorld->setGravity( btVector3( 0., 0., -32.14 ) );

    return( dynamicsWorld );
}


int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    const bool buildWorld( arguments.read( "--build" ) );

    std::string mapFile( "" );
    arguments.read( "--map", mapFile );

    const bool debugDisplay( arguments.read( "--debug" ) );


    btDynamicsWorld* bw = initPhysics();

    osg::Group* root = new osg::Group;
    if( buildWorld )
        root->addChild( build() );
    else
        root->addChild( osgDB::readNodeFiles( arguments ) );
    if( root->getNumChildren() == 0 )
    {
        osg::notify( osg::FATAL ) << "No world to render." << std::endl;
        return( 1 );
    }

    EnablePhysicsVisitor epv( bw );
    root->accept( epv );

    // Add character.
    Character worker( bw );
    osg::Node* modelRoot = worker.setModel( "ps-worker.osg" );
    if( modelRoot == NULL ) return( 1 );
    root->addChild( modelRoot );

    osg::Group* launchHandlerAttachPoint = new osg::Group;
    root->addChild( launchHandlerAttachPoint );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 450 );
    viewer.setSceneData( root );
    viewer.getCamera()->setDataVariance( osg::Object::DYNAMIC );

#ifdef DIRECTINPUT_ENABLED
    osg::ref_ptr< osgwMx::MxGamePadDX > workerControl = new osgwMx::MxGamePadDX;
    workerControl->setStickRate( 5. );

	// Load the optional functional mapping, if specified.
	if( !( mapFile.empty() ) )
	{
		osgwMx::FunctionalMap* map = dynamic_cast< osgwMx::FunctionalMap* >(
			osgDB::readObjectFile( mapFile ) );
		if( map != NULL )
			workerControl->setFunctionalMap( map );
		else
			osg::notify( osg::WARN ) << "Unable to load map file \"" << mapFile << "\"" << std::endl;
	}
#else
    osg::ref_ptr< osgwMx::MxEventHandler > workerControl = new osgwMx::MxEventHandler;
    viewer.addEventHandler( workerControl.get() );
#endif

    osgwMx::MxCore* mxCore = workerControl->getMxCore();
    mxCore->setInitialValues( osg::Vec3( 0., 0., 1. ), osg::Vec3( 0., 1., 0. ),
        osg::Vec3( 0., 0., 5. ) );
    mxCore->reset();

    osg::ref_ptr< KeyHandler > keyHandler = new KeyHandler( mxCore );
    viewer.addEventHandler( keyHandler.get() );

    osgbInteraction::LaunchHandler* lh = new osgbInteraction::LaunchHandler(
        bw, launchHandlerAttachPoint, viewer.getCamera() );
    {
        // Use a custom launch model: Sphere with radius 0.2 (instead of default 1.0).
        osg::Geode* geode = new osg::Geode;
        const double radius( .2 );
        geode->addDrawable( osgwTools::makeGeodesicSphere( radius ) );
        lh->setLaunchModel( geode, new btSphereShape( radius ) );
        lh->setInitialVelocity( 40. );
    }
    viewer.addEventHandler( lh );

    osgbCollision::GLDebugDrawer* dbgDraw( NULL );
    if( debugDisplay )
    {
        dbgDraw = new osgbCollision::GLDebugDrawer();
        dbgDraw->setDebugMode( ~btIDebugDraw::DBG_DrawText );
        bw->setDebugDrawer( dbgDraw );
        root->addChild( dbgDraw->getSceneGraph() );
    }

    viewer.frame();
    double prevSimTime = 0.;
    while( !( viewer.done() ) )
    {
        viewer.advance();
        const double currSimTime = viewer.getFrameStamp()->getSimulationTime();
        const double deltaTime = currSimTime - prevSimTime;
        prevSimTime = currSimTime;

        // Get events from either the game pad or the kbd/mouse to control the
        // worker and update the MxCore.
#ifdef DIRECTINPUT_ENABLED
        workerControl->poll( deltaTime );
#endif
        viewer.eventTraversal();

        // Set the target position for the worker from the MxCore.
        worker.setPhysicsWorldTransform( mxCore->getMatrix() );
        //osg::notify( osg::ALWAYS ) << worker.getPosition() << std::endl;

        // Run the physics simultation.
        if( dbgDraw != NULL )
            dbgDraw->BeginDraw();
        bw->stepSimulation( deltaTime );
        if( dbgDraw != NULL )
        {
            bw->debugDrawWorld();
            dbgDraw->EndDraw();
        }

        // The physics simultation has now adjusted the position of the
        // worker. Set the MxCore position from the worker position.
        mxCore->setPosition( worker.getPosition() );
        // Now update the worker's OSG transform, and the OSG Camera.
        worker.setOSGMatrix( mxCore->getMatrix() );
        keyHandler->setViewMatrix( viewer.getCamera() );

        viewer.updateTraversal();
        viewer.renderingTraversals();
    }
}
