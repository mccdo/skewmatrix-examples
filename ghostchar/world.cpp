// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include "world.h"

#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/NodeVisitor>

#include <osgwTools/AbsoluteModelTransform.h>
#include <osgwTools/Shapes.h>
#include <osgbDynamics/RigidBody.h>
#include <osgbDynamics/CreationRecord.h>



osg::Object* makeCreationRecord( osg::Transform* node, BroadphaseNativeTypes shape,
    double mass )
{
    osg::ref_ptr< osgbDynamics::CreationRecord > cr = new osgbDynamics::CreationRecord;

    cr->_sceneGraph = node;
    cr->_shapeType = shape;
    cr->_mass = mass;

    return( cr.release() );
}


osg::Node* duckHazard( float x, float y, float height )
{
    osgwTools::AbsoluteModelTransform* amt;
    osg::Geode* geode;
    osg::Matrix m;

    osg::Group* root = new osg::Group;

    amt = new osgwTools::AbsoluteModelTransform;
    geode = new osg::Geode;
    m = osg::Matrix::translate( x-2., y, 1. + ( height * .5 ) );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( .125, .125, height * .5 ), osg::Vec3s( 1, 1, (short)height ) ) );
    amt->addChild( geode );
    root->addChild( amt );
    amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

    amt = new osgwTools::AbsoluteModelTransform;
    geode = new osg::Geode;
    m = osg::Matrix::translate( x+2., y, 1. + ( height * .5 ) );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( .125, .125, height * .5 ), osg::Vec3s( 1, 1, (short)height ) ) );
    amt->addChild( geode );
    root->addChild( amt );
    amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

    amt = new osgwTools::AbsoluteModelTransform;
    geode = new osg::Geode;
    m = osg::Matrix::translate( x, y, height + 1.2 );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 2.5, .2, .2 ), osg::Vec3s( 3, 1, 1 ) ) );
    amt->addChild( geode );
    root->addChild( amt );
    amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, .5 ) );

    return( root );
}


osg::Node* build()
{
    osg::ref_ptr< osg::Group > root = new osg::Group;

    osgwTools::AbsoluteModelTransform* amt = new osgwTools::AbsoluteModelTransform;
    osg::Geode* geode = new osg::Geode;
    osg::Matrix m = osg::Matrix::translate( 0., 0., .5 );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 50., 50., .5 ), osg::Vec3s( 100, 100, 1 ) ) );
    amt->addChild( geode );
    root->addChild( amt );
    amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );


    // Stairs
    {
        osg::Group* stairsGroup = new osg::Group;
        root->addChild( stairsGroup );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -25., -25., 1.25 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 20., 20., .25 ), osg::Vec3s( 40, 40, 1 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -25., -25., 1.75 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 19., 19., .25 ), osg::Vec3s( 38, 38, 1 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -25., -25., 2.25 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 18., 18., .25 ), osg::Vec3s( 36, 36, 1 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -25., -25., 2.75 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 17., 17., .25 ), osg::Vec3s( 34, 34, 1 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -25., -25., 3.25 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 16., 16., .25 ), osg::Vec3s( 32, 32, 1 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -25., -25., 3.75 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 15., 15., .25 ), osg::Vec3s( 30, 30, 1 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );


        // Floor height at top of stairs platform is 3.5.
        // Put some dynamic objects up here to walk into.
        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -29., -20., 6.5 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( .5, .5, 3. ), osg::Vec3s( 1, 1, 2 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 3. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -25., -20., 6.5 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( .5, .5, 3. ), osg::Vec3s( 1, 1, 2 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 3. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -27., -20., 9.75 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 2., .5, .5 ), osg::Vec3s( 1, 1, 2 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 1. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -21., -20., 6.5 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( .5, .5, 3. ), osg::Vec3s( 1, 1, 2 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 3. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -23., -20., 9.75 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 2., .5, .5 ), osg::Vec3s( 1, 1, 2 ) ) );
        amt->addChild( geode );
        stairsGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 1. ) );
    }


    // Wall pile
    {
        osg::Group* wallGroup = new osg::Group;
        root->addChild( wallGroup );

        float xPos( 10. ), zPos( 1.6 );
        int xCount( 5 );
        int xIdx;
        float mass( 2. );
        while( xCount > 0 )
        {
            for( xIdx=0; xIdx<xCount; xIdx++ )
            {
                amt = new osgwTools::AbsoluteModelTransform;
                geode = new osg::Geode;
                m = osg::Matrix::translate( xPos, -10., zPos );
                geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 1., .35, .6 ) ) );
                amt->addChild( geode );
                wallGroup->addChild( amt );
                amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, mass ) );
                xPos += 2.;
            }
            xPos = xPos - ( 2. * xCount ) + 1.;
            zPos += 1.2;
            xCount--;
            mass *= .9;
        }
    }


    // Small maze
    {
        osg::Group* mazeGroup = new osg::Group;
        root->addChild( mazeGroup );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -5., 38., 4. );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( .25, 2., 3. ), osg::Vec3s( 1, 2, 3 ) ) );
        amt->addChild( geode );
        mazeGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -11., 40., 4. );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 6., .25, 3. ), osg::Vec3s( 6, 1, 3 ) ) );
        amt->addChild( geode );
        mazeGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -11., 36., 4. );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 2., .25, 3. ), osg::Vec3s( 2, 1, 3 ) ) );
        amt->addChild( geode );
        mazeGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -17., 36., 4. );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( .25, 4., 3. ), osg::Vec3s( 1, 4, 3 ) ) );
        amt->addChild( geode );
        mazeGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -13., 32., 4. );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( .25, 4., 3. ), osg::Vec3s( 1, 4, 3 ) ) );
        amt->addChild( geode );
        mazeGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -11., 28., 4. );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 6., .25, 3. ), osg::Vec3s( 6, 1, 3 ) ) );
        amt->addChild( geode );
        mazeGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -7., 32., 4. );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 2., .25, 3. ), osg::Vec3s( 2, 1, 3 ) ) );
        amt->addChild( geode );
        mazeGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( -5., 30., 4. );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( .25, 2., 3. ), osg::Vec3s( 1, 2, 3 ) ) );
        amt->addChild( geode );
        mazeGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        // Decorative, not a collision shape
        geode = new osg::Geode;
        m = osg::Matrix::translate( -5., 28., 1. );
        geode->addDrawable( osgwTools::makeClosedCylinder( m, 6., .25, .25, false, true, osg::Vec2s( 3, 16 ) ) );
        mazeGroup->addChild( geode );

        geode = new osg::Geode;
        m = osg::Matrix::translate( -5., 32., 1. );
        geode->addDrawable( osgwTools::makeClosedCylinder( m, 6., .25, .25, false, true, osg::Vec2s( 3, 16 ) ) );
        mazeGroup->addChild( geode );

        geode = new osg::Geode;
        m = osg::Matrix::translate( -5., 40., 1. );
        geode->addDrawable( osgwTools::makeClosedCylinder( m, 6., .25, .25, false, true, osg::Vec2s( 3, 16 ) ) );
        mazeGroup->addChild( geode );

        geode = new osg::Geode;
        m = osg::Matrix::translate( -17., 40., 1. );
        geode->addDrawable( osgwTools::makeClosedCylinder( m, 6., .25, .25, false, true, osg::Vec2s( 3, 16 ) ) );
        mazeGroup->addChild( geode );

        geode = new osg::Geode;
        m = osg::Matrix::translate( -13., 36., 1. );
        geode->addDrawable( osgwTools::makeClosedCylinder( m, 6., .25, .25, false, true, osg::Vec2s( 3, 16 ) ) );
        mazeGroup->addChild( geode );
    }


    // Dominoes
    {
        osg::Group* dominoesGroup = new osg::Group;
        root->addChild( dominoesGroup );

        float xPos( -30. ), yPos( 5. ), ySpace( 3. );
        int yCount( 10 );
        while( yCount-- > 0 )
        {
            amt = new osgwTools::AbsoluteModelTransform;
            geode = new osg::Geode;
            m = osg::Matrix::translate( xPos, yPos, 4.5 );
            geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 1.5, .25, 3.5 ) ) );
            amt->addChild( geode );
            dominoesGroup->addChild( amt );
            amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 3. ) );
            yPos += ySpace;
        }
    }


    // Fulcrum / lever
    {
        osg::Group* fulcrumGroup = new osg::Group;
        root->addChild( fulcrumGroup );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( 30., 10., 1.5 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 1.55, .25, .5 ) ) );
        amt->addChild( geode );
        fulcrumGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 0. ) );

        amt = new osgwTools::AbsoluteModelTransform;
        geode = new osg::Geode;
        m = osg::Matrix::translate( 30., 9, 2.5 );
        geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 1.5, 6., .25 ), osg::Vec3s( 1, 6, 1 ) ) );
        amt->addChild( geode );
        fulcrumGroup->addChild( amt );
        amt->setUserData( makeCreationRecord( amt, BOX_SHAPE_PROXYTYPE, 1. ) );
    }


    // Duck hazards
    {
        osg::Group* duckGroup = new osg::Group;
        root->addChild( duckGroup );

        duckGroup->addChild( duckHazard( 10., 30., 4. ) );
        duckGroup->addChild( duckHazard( 10., 20., 5. ) );
        duckGroup->addChild( duckHazard( 10., 10., 6. ) );
    }


    // Debug: wire grid to make movement plain and obvious.
    geode = new osg::Geode;
    geode->addDrawable( osgwTools::makeWirePlane( osg::Vec3( -40., -40., 1.1 ), osg::Vec3( 80., 0., 0. ),
        osg::Vec3( 0., 80., 0. ), osg::Vec2s( 16, 16 ) ) );
    root->addChild( geode );

    return( root.release() );
}



EnablePhysicsVisitor::EnablePhysicsVisitor( btDynamicsWorld* bw )
  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
    _bw( bw )
{
}
EnablePhysicsVisitor::~EnablePhysicsVisitor()
{
}

void EnablePhysicsVisitor::apply( osg::Transform& node )
{
    osg::ref_ptr< osgbDynamics::CreationRecord > cr = dynamic_cast<
        osgbDynamics::CreationRecord* >( node.getUserData() );
    if( !( cr.valid() ) )
    {
        traverse( node );
        return;
    }

    osg::NodePath np = getNodePath();
    np.pop_back();
    cr->_parentTransform = osg::computeLocalToWorld( np );
    btRigidBody* rb = osgbDynamics::createRigidBody( cr.get() );

    rb->setSleepingThresholds( .05, .025 );

    //node.setUserData( new osgbCollision::RefRigidBody( rb ) );

    short group, mask;
    if( cr->_mass == 0. )
    {
        group = btBroadphaseProxy::StaticFilter;
        mask = btBroadphaseProxy::CharacterFilter | btBroadphaseProxy::DefaultFilter;
        rb->setCollisionFlags( btCollisionObject::CF_STATIC_OBJECT );
    }
    else
    {
        group = btBroadphaseProxy::DefaultFilter;
        mask = btBroadphaseProxy::CharacterFilter | btBroadphaseProxy::StaticFilter;
    }

    _bw->addRigidBody( rb );//, group, mask );

    traverse( node );
    return;
}

