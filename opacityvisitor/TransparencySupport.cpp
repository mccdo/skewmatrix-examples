// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include "TransparencySupport.h"
#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/NodeVisitor>
#include <osg/Geode>


// The OSG convention is bin number 10. We want to pick
// a slightly different number, whoch would be unlikely that
// any other code would use it. This helps us detect our
// own transparent StateSet, thereby avoiding setting other
// transparent StateSets to opaque.
// OK, it's kind of a hack.
static int s_magicBinNumber( 10 );

static std::string s_magicStateSetName( "TransparentDeleteMe" );


bool transparentEnable( osg::Node* node, float alpha )
{
    if( node == NULL )
        return( false );

    osg::StateSet* stateSet( node->getStateSet() );
    if( ( stateSet != NULL ) &&
        ( node->getUserData() == NULL ) )
    {
        // We have a StateSet, and UserData is NULL, so make a copy of the StateSet.
        // We'll store the original StateSet as UserData (for later restore) and modify
        // the copy.
        node->setUserData( stateSet );
        stateSet = new osg::StateSet( *( stateSet ), osg::CopyOp::DEEP_COPY_ALL );
        node->setStateSet( stateSet );
    }
    else if( stateSet == NULL )
    {
        // This node doesn't have a StateSet, so we create one and tag it
        // with the magic name for later deletion.
        stateSet = new osg::StateSet();
        stateSet->setName( s_magicStateSetName );
        node->setStateSet( stateSet );
    }

    if( !( transparentEnable( stateSet, alpha ) ) )
        return( false );

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

    osg::Referenced* userData = node->getUserData();
    osg::StateSet* origStateSet = dynamic_cast< osg::StateSet* >( userData );
    if( origStateSet == NULL )
    {
        // Probably the node had something else attached to UserData, so we
        // were unable to save the StateSet and had to modify the attached StateSet.
        // There's no way to restore the StateSet to its original state in this case.
        osg::StateSet* stateSet = node->getStateSet();
        if( stateSet->getName() == s_magicStateSetName )
            node->setStateSet( NULL );
        else
        {
            stateSet->removeAttribute( osg::StateAttribute::BLENDCOLOR );
            stateSet->removeAttribute( osg::StateAttribute::BLENDFUNC );
            stateSet->removeMode( GL_BLEND );
            stateSet->setRenderingHint( osg::StateSet::DEFAULT_BIN );
        }
    }
    else
    {
        node->setStateSet( origStateSet );
        node->setUserData( NULL );
    }

    return( true );
}

bool isTransparent( osg::Node* node )
{
    osg::StateSet* stateSet = node->getStateSet();
    if( stateSet != NULL )
    {
        return( isTransparent( stateSet ) );
    }
    else
    {
        return( false );
    }
}

bool transparentEnable( osg::StateSet* stateSet, float alpha )
{
    const osg::StateAttribute::GLModeValue modeValue = 
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE;

    osg::BlendColor* bc = new osg::BlendColor( osg::Vec4( 0., 0., 0., alpha ) );
    stateSet->setAttributeAndModes( bc, modeValue );
    osg::BlendFunc* bf = new osg::BlendFunc( osg::BlendFunc::CONSTANT_ALPHA,
        osg::BlendFunc::ONE_MINUS_CONSTANT_ALPHA );
    stateSet->setAttributeAndModes( bf, modeValue );
    stateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    stateSet->setBinNumber( s_magicBinNumber );

    return( true );
}

bool transparentDisable( osg::Drawable* drawable )
{
    if( drawable == NULL )
        return( false );

    osg::StateSet* stateSet = drawable->getStateSet();
    if( stateSet == NULL )
        // We don't have a StateSet, so we are not transparent.
        return( false );
    if( !isTransparent( stateSet ) )
        // We have a StateSet, but it isn't transparent.
        return( false );

    osg::Referenced* userData = drawable->getUserData();
    osg::StateSet* origStateSet = dynamic_cast< osg::StateSet* >( userData );
    if( origStateSet == NULL )
    {
        // Probably the drawable had something else attached to UserData, so we
        // were unable to save the StateSet and had to modify the attached StateSet.
        // There's no way to restore the StateSet to its original state in this case.
        osg::StateSet* stateSet = drawable->getStateSet();
        if( stateSet->getName() == s_magicStateSetName )
            drawable->setStateSet( NULL );
        else
        {
            stateSet->removeAttribute( osg::StateAttribute::BLENDCOLOR );
            stateSet->removeAttribute( osg::StateAttribute::BLENDFUNC );
            stateSet->removeMode( GL_BLEND );
            stateSet->setRenderingHint( osg::StateSet::DEFAULT_BIN );
        }
    }
    else
    {
        drawable->setStateSet( origStateSet );
        drawable->setUserData( NULL );
    }

    return( true );
}

bool isTransparent( osg::StateSet* stateSet )
{
    const bool hasBlendColor = ( stateSet->getAttribute( osg::StateAttribute::BLENDCOLOR ) != NULL );
    const bool hasBlendFunc = ( stateSet->getAttribute( osg::StateAttribute::BLENDFUNC ) != NULL );
    const bool blendEnabled = ( ( stateSet->getMode( GL_BLEND ) & osg::StateAttribute::ON ) != 0 );
    const bool hasRenderingHint = ( stateSet->getRenderingHint() == osg::StateSet::TRANSPARENT_BIN );
    const bool hasMagicBinNumber = ( stateSet->getBinNumber() == s_magicBinNumber );

    return( hasBlendColor && hasBlendFunc && blendEnabled && hasRenderingHint && hasMagicBinNumber );
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

void RestoreOpacityVisitor::apply( osg::Node& node )
{
    if( isTransparent( &node ) )
        transparentDisable( &node );

    traverse( node );
}
void RestoreOpacityVisitor::apply( osg::Geode& geode )

{
    if( isTransparent( &geode ) )
        transparentDisable( &geode );

    unsigned int idx;
    for( idx=0; idx<geode.getNumDrawables(); idx++ )
    {
        transparentDisable( geode.getDrawable( idx ) );
    }

    traverse( geode );
}
