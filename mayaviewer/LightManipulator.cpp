// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.


#include "LightManipulator.h"
#include <osgwTools/Shapes.h>
#include <osg/MatrixTransform>
#include <osg/LightSource>
#include <osg/Geode>


LightManipulator::LightManipulator()
  : _scale( 1.f )
{
    osg::MatrixTransform* mt = new osg::MatrixTransform;
    _lightSubgraph = mt;
    mt->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    osg::LightSource* ls = new osg::LightSource;
    mt->addChild( ls );

    osg::Light* light = ls->getLight();
    light->setLightNum( 0 );
    light->setPosition( osg::Vec4( 0., 0., 0., 1. ) );

    osg::Geode* geode = new osg::Geode;
    mt->addChild( geode );
    geode->addDrawable( osgwTools::makeGeodesicSphere( 10.f ) );
}
LightManipulator::~LightManipulator()
{
}

bool LightManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    bool handled( false );
    osg::Vec3 delta;

    switch( ea.getEventType() )
    {
    case( osgGA::GUIEventAdapter::KEYDOWN ):
    {
        switch( ea.getKey() )
        {
        case( 'a' ):
            delta.set( -1., 0., 0. );
            handled = true;
            break;
        case( 's' ):
            delta.set( 1., 0., 0. );
            handled = true;
            break;
        case( 'd' ):
            delta.set( 0., -1., 0. );
            handled = true;
            break;
        case( 'f' ):
            delta.set( 0., 1., 0. );
            handled = true;
            break;
        case( 'e' ):
            delta.set( 0., 0., 1. );
            handled = true;
            break;
        case( 'c' ):
            delta.set( 0., 0., -1. );
            handled = true;
            break;
        case( '+' ):
            _scale *= 1.2f;
            handled = true;
            break;
        case( '-' ):
            _scale /= 1.2f;
            handled = true;
            break;
        }
    }
    }

    if( handled )
    {
        _position += ( delta * _scale );

        osg::MatrixTransform* mt = static_cast< osg::MatrixTransform* >( _lightSubgraph.get() );
        mt->setMatrix( osg::Matrix::translate( _position ) );
    }

    return( handled );
}

osg::Node* LightManipulator::getLightSubgraph()
{
    return( _lightSubgraph.get() );
}
