// Copyright (c) 2011 Skew Matrix Software LLC. All rights reserved.

#include "world.h"

#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/Geode>

#include <osgwTools/AbsoluteModelTransform.h>
#include <osgwTools/Shapes.h>
#include <osgbDynamics/CreationRecord.h>



osg::Object* makeCreationRecord( osg::Transform* node, BroadphaseNativeTypes shape,
    double mass, const osg::NodePath& np )
{
    osg::ref_ptr< osgbDynamics::CreationRecord > cr = new osgbDynamics::CreationRecord;

    cr->_sceneGraph = node;
    cr->_shapeType = shape;
    cr->_mass = mass;
    cr->_parentTransform = osg::computeLocalToWorld( np );

    return( cr.release() );
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

    // Stairs
    amt = new osgwTools::AbsoluteModelTransform;
    geode = new osg::Geode;
    m = osg::Matrix::translate( -25., -25., 1.25 );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 20., 20., .25 ), osg::Vec3s( 40, 40, 1 ) ) );
    amt->addChild( geode );
    root->addChild( amt );

    amt = new osgwTools::AbsoluteModelTransform;
    geode = new osg::Geode;
    m = osg::Matrix::translate( -25., -25., 1.75 );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 19., 19., .25 ), osg::Vec3s( 38, 38, 1 ) ) );
    amt->addChild( geode );
    root->addChild( amt );

    amt = new osgwTools::AbsoluteModelTransform;
    geode = new osg::Geode;
    m = osg::Matrix::translate( -25., -25., 2.25 );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 18., 18., .25 ), osg::Vec3s( 36, 36, 1 ) ) );
    amt->addChild( geode );
    root->addChild( amt );

    amt = new osgwTools::AbsoluteModelTransform;
    geode = new osg::Geode;
    m = osg::Matrix::translate( -25., -25., 2.75 );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 17., 17., .25 ), osg::Vec3s( 34, 34, 1 ) ) );
    amt->addChild( geode );
    root->addChild( amt );

    amt = new osgwTools::AbsoluteModelTransform;
    geode = new osg::Geode;
    m = osg::Matrix::translate( -25., -25., 3.25 );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 16., 16., .25 ), osg::Vec3s( 32, 32, 1 ) ) );
    amt->addChild( geode );
    root->addChild( amt );

    amt = new osgwTools::AbsoluteModelTransform;
    geode = new osg::Geode;
    m = osg::Matrix::translate( -25., -25., 3.75 );
    geode->addDrawable( osgwTools::makeBox( m, osg::Vec3( 15., 15., .25 ), osg::Vec3s( 30, 30, 1 ) ) );
    amt->addChild( geode );
    root->addChild( amt );

    // Floor height at top of stairs platform is 3.5.
    // Put some dynamic objects up here to walk into.


    return( root.release() );
}
