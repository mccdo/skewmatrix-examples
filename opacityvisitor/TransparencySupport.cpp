// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include "TransparencySupport.h"
#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/NodeVisitor>
#include <osg/Geode>


bool transparentEnable( osg::Node* node, float alpha )
{
    if( node == NULL )
        return( false );

    osg::StateSet* stateSet( NULL );
    if( ( node->getStateSet() != NULL ) &&
        ( node->getUserData() == NULL ) )
    {
        // We have a StateSet, and UserData is NULL, so make a copy of the StateSet.
        // We'll store the original StateSet as UserData (for later restore) and modify
        // the copy.
        stateSet = new osg::StateSet( *( node->getStateSet() ), osg::CopyOp::DEEP_COPY_ALL );
        node->setUserData( node->getStateSet() );
    }
    else
    {
        stateSet = new osg::StateSet();
    }

    osg::BlendColor* bc = new osg::BlendColor( osg::Vec4( 0., 0., 0., alpha ) );
    stateSet->setAttributeAndModes( bc, osg::StateAttribute::ON );
    osg::BlendFunc* bf = new osg::BlendFunc( osg::BlendFunc::CONSTANT_ALPHA,
        osg::BlendFunc::ONE_MINUS_CONSTANT_ALPHA );
    stateSet->setAttributeAndModes( bf, osg::StateAttribute::ON );
    stateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    node->setStateSet( stateSet );
    return( true );
}

bool transparentDisable( osg::Node* node, bool recursive )
{
    if( node == NULL )
        return( false );

    if( recursive )
    {
        RestoreOpacityVisitor rov;
        node->accept( rov );
        return( true );
    }

    if( !isTransparent( node ) )
        return( false );

    osg::StateSet* origStateSet = dynamic_cast< osg::StateSet* >( node->getUserData() );
    node->setStateSet( origStateSet );
    node->setUserData( NULL );

    return( true );
}

bool isTransparent( osg::Node* node )
{
    osg::StateSet* stateSet = node->getStateSet();
    return( ( stateSet != NULL ) &&
            ( stateSet->getRenderingHint() == osg::StateSet::TRANSPARENT_BIN ) );
}




ProtectTransparencyVisitor::ProtectTransparencyVisitor()
  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
}
ProtectTransparencyVisitor::~ProtectTransparencyVisitor()
{
}

void ProtectTransparencyVisitor::apply( osg::Node& node )
{
    traverse( node );
}
void ProtectTransparencyVisitor::apply( osg::Geode& geode )
{
    unsigned int idx;
    for( idx=0; idx<geode.getNumDrawables(); idx++ )
    {
    }

    traverse( geode );
}


RestoreOpacityVisitor::RestoreOpacityVisitor()
  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
}
RestoreOpacityVisitor::~RestoreOpacityVisitor()
{
}

void RestoreOpacityVisitor::apply( osg::Node& node )
{
    traverse( node );
}
void RestoreOpacityVisitor::apply( osg::Geode& geode )
{
    unsigned int idx;
    for( idx=0; idx<geode.getNumDrawables(); idx++ )
    {
    }

    traverse( geode );
}
