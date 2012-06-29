// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.


#include "LightManipulator.h"
#include <osgwTools/Shapes.h>
#include <osg/MatrixTransform>
#include <osg/LightSource>
#include <osg/Geode>


LightManipulator::LightManipulator( const float lightSize )
  : _lightSize( lightSize ),
    _scale( 1.f )
{
    osg::MatrixTransform* mt = new osg::MatrixTransform;
    _lightSubgraph = mt;
    mt->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    osg::LightSource* ls = new osg::LightSource;
    mt->addChild( ls );

    osg::Light* light = ls->getLight();
    light->setLightNum( 0 );
    light->setPosition( osg::Vec4( 0., 0., 0., 1. ) );
    light->setDiffuse( osg::Vec4( .5, .5, .5, 1. ) );
    light->setSpecular( osg::Vec4( 1., 1., 1., 1. ) );

    if( _lightSize > 0. )
    {
        osg::Geode* geode = new osg::Geode;
        mt->addChild( geode );
        geode->addDrawable( osgwTools::makeGeodesicSphere( _lightSize ) );
    }
}
LightManipulator::~LightManipulator()
{
}

bool LightManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    const bool ctrl( ( ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL ) != 0 );

    bool handled( false );
    osg::Vec3 delta;

    switch( ea.getEventType() )
    {
    case( osgGA::GUIEventAdapter::KEYDOWN ):
    {
        // Hm. Why do we need "| 0x60" here??
        switch( ea.getKey() | 0x60 )
        {
        case( 'a' ):
            if( ctrl )
            {
                delta.set( -1., 0., 0. );
                handled = true;
            }
            break;
        case( 's' ):
            if( ctrl )
            {
                delta.set( 1., 0., 0. );
                handled = true;
            }
            break;
        case( 'd' ):
            if( ctrl )
            {
                delta.set( 0., -1., 0. );
                handled = true;
            }
            break;
        case( 'f' ):
            if( ctrl )
            {
                delta.set( 0., 1., 0. );
                handled = true;
            }
            break;
        case( 'e' ):
            if( ctrl )
            {
                delta.set( 0., 0., 1. );
                handled = true;
            }
            break;
        case( 'c' ):
            if( ctrl )
            {
                delta.set( 0., 0., -1. );
                handled = true;
            }
            break;
        }
        switch( ea.getKey() )
        {
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
