// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/GUIEventHandler>
#include <osg/MatrixTransform>
#include <osg/Geode>

#include <osgwTools/AbsoluteModelTransform.h>
#include <osgwTools/Shapes.h>
#include <osgbDynamics/CreationRecord.h>

#ifdef DIRECTINPUT_ENABLED
#include <osgwMx/MxGamePadDX.h>
#else
#include <osgwMx/MxEventHandler.h>
#endif

#include "world.h"
#include "character.h"


/** \cond */
class KeyHandler : public osgGA::GUIEventHandler
{
public:
    enum ViewMode {
        LOCAL,
        FOLLOW,
        GLOBAL
    };

    KeyHandler( osg::Camera* camera, osgwMx::MxCore* mxCore )
      : _camera( camera ),
        _mxCore( mxCore ),
        _viewMode( LOCAL )
    {
        _localOffsetEC = osg::Matrix::translate( 0., -5.5, .5 ) *
            osg::Matrix::rotate( .05, 1., 0., 0. );
        _followOffsetEC = osg::Matrix::translate( 0., -10., -11. ) *
            osg::Matrix::rotate( .5, 1., 0., 0. );
        osg::Vec3 dir( 0., 10., -5. );
        dir.normalize();
        _globalDirWC = dir * 50.;
    }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
    {
        bool handled( false );
        switch( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::FRAME:
        {
            switch( _viewMode )
            {
            case LOCAL:
                _camera->setViewMatrix( _mxCore->getInverseMatrix() * _localOffsetEC );
                handled = true;
                break;
            case FOLLOW:
                _camera->setViewMatrix( _mxCore->getInverseMatrix() * _followOffsetEC );
                handled = true;
                break;
            case GLOBAL:
                const osg::Vec3 pos = _mxCore->getPosition();
                const osg::Vec3 eye = pos - _globalDirWC;
                _camera->setViewMatrix( osg::Matrix::lookAt( eye, pos, osg::Vec3( 0., 0., 1. ) ) );
                handled = true;
                break;
            }
            break;
        }
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

    osg::ref_ptr< osg::Camera > _camera;
    osg::ref_ptr< osgwMx::MxCore > _mxCore;
    ViewMode _viewMode;

    osg::Matrix _localOffsetEC;
    osg::Matrix _followOffsetEC;
    osg::Vec3 _globalDirWC;
};
/** \endcond */


btDynamicsWorld* initPhysics()
{
    btDefaultCollisionConfiguration * collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher * dispatcher = new btCollisionDispatcher( collisionConfiguration );
    btConstraintSolver * solver = new btSequentialImpulseConstraintSolver;

    btVector3 worldAabbMin( -10000, -10000, -10000 );
    btVector3 worldAabbMax( 10000, 10000, 10000 );
    btBroadphaseInterface* inter = new btAxisSweep3( worldAabbMin, worldAabbMax, 1000 );

    btDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld( dispatcher, inter, solver, collisionConfiguration );

    // Set gravity in ft/sec^2: accl due to gravity is ~9.8 m/s^2 * 3.28 ft/m = 32.14
    dynamicsWorld->setGravity( btVector3( 0., 0., -32.14 ) );

    return( dynamicsWorld );
}


int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    const bool buildWorld( arguments.read( "--build" ) );

    std::string mapFile( "" );
    arguments.read( "--map", mapFile );


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
    Character worker;
    osg::Node* modelRoot = worker.setModel( "ps-worker.osg" );
    if( modelRoot == NULL ) return( 1 );
    root->addChild( modelRoot );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 450 );
    viewer.setSceneData( root );
    viewer.getCamera()->setDataVariance( osg::Object::DYNAMIC );

#ifdef DIRECTINPUT_ENABLED
    osg::ref_ptr< osgwMx::MxGamePadDX > workerControl = new osgwMx::MxGamePadDX;

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
    viewer.addEventHandler( new KeyHandler( viewer.getCamera(), mxCore ) );

    viewer.frame();
    double prevSimTime = 0.;
    while( !( viewer.done() ) )
    {
        viewer.advance();
        const double currSimTime = viewer.getFrameStamp()->getSimulationTime();

        // Handle events before setting worker matrix. This means workerControl
        // handles its events, configures the MxCore, then KeyHandler FRAME and
        // the construction worker will get the same (updated) matrix.
        viewer.eventTraversal();
        workerControl->poll( currSimTime - prevSimTime );

        worker.setMatrix( mxCore->getMatrix() );

        bw->stepSimulation( currSimTime - prevSimTime );

        viewer.updateTraversal();
        viewer.renderingTraversals();

        prevSimTime = currSimTime;
    }
}
