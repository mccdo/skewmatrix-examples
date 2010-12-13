// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include "TransparencySupport.h"
#include <osg/BlendColor>
#include <osg/BlendFunc>
#include <osg/NodeVisitor>
#include <osg/Geode>


// When enabling transparency on a Node or Drawable that has no StateSet,
// we addign this name to the Newly created StateSet. When transparency
// is later disabled, if the name matches, we delete the StateSet.
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
        osg::StateSet* stateSet = node->getStateSet();
        if( stateSet->getName() == s_magicStateSetName )
        {
            // We created the StateSet, so just delete it.
            node->setStateSet( NULL );
        }
        else
        {
            // We didn't create this StateSet, and we weren't able to save it.
            // Best thing we can do is delete the state we added and hope we haven't
            // damaged anything.
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

bool isTransparent( const osg::Node* node )
{
    const osg::StateSet* stateSet = node->getStateSet();
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
        osg::StateSet* stateSet = drawable->getStateSet();
        if( stateSet->getName() == s_magicStateSetName )
        {
            // We created the StateSet, so just delete it.
            drawable->setStateSet( NULL );
        }
        else
        {
            // We didn't create this StateSet, and we weren't able to save it.
            // Best thing we can do is delete the state we added and hope we haven't
            // damaged anything.
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

bool isTransparent( const osg::StateSet* stateSet )
{
    const bool hasBlendColor = ( stateSet->getAttribute( osg::StateAttribute::BLENDCOLOR ) != NULL );
    const bool hasBlendFunc = ( stateSet->getAttribute( osg::StateAttribute::BLENDFUNC ) != NULL );
    const bool blendEnabled = ( ( stateSet->getMode( GL_BLEND ) & osg::StateAttribute::ON ) != 0 );
    const bool hasRenderingHint = ( stateSet->getRenderingHint() == osg::StateSet::TRANSPARENT_BIN );

    return( hasBlendColor && hasBlendFunc && blendEnabled && hasRenderingHint );
}




ProtectTransparencyVisitor::ProtectTransparencyVisitor()
  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
}

void ProtectTransparencyVisitor::apply( osg::Node& node )
{
    protectTransparent( node.getStateSet() );
    traverse( node );
}

void ProtectTransparencyVisitor::apply( osg::Geode& geode )
{
    protectTransparent( geode.getStateSet() );

    unsigned int idx;
    for( idx=0; idx<geode.getNumDrawables(); idx++ )
    {
        protectTransparent( geode.getDrawable( idx )->getStateSet() );
    }

    traverse( geode );
}

void ProtectTransparencyVisitor::protectTransparent( osg::StateSet* stateSet ) const
{
    if( stateSet == NULL )
    {
        return;
    }

    if( isTransparentInternal( stateSet ) )
    {
        stateSet->setMode( GL_BLEND, stateSet->getMode( GL_BLEND ) | osg::StateAttribute::PROTECTED );

        osg::BlendColor* bc = dynamic_cast< osg::BlendColor* >( stateSet->getAttribute( osg::StateAttribute::BLENDCOLOR ) );
        if( bc != NULL )
            stateSet->setAttributeAndModes( bc, stateSet->getMode( GL_BLEND ) | osg::StateAttribute::PROTECTED );

        osg::BlendFunc* bf = dynamic_cast< osg::BlendFunc* >( stateSet->getAttribute( osg::StateAttribute::BLENDFUNC ) );
        if( bf != NULL )
            stateSet->setAttributeAndModes( bf, stateSet->getMode( GL_BLEND ) | osg::StateAttribute::PROTECTED );
    }
}

bool ProtectTransparencyVisitor::isTransparentInternal( const osg::StateSet* stateSet ) const
{
    bool blendEnabled = ( ( stateSet->getMode( GL_BLEND ) & osg::StateAttribute::ON ) != 0 );
    bool hasTranslucentTexture = false;
    bool hasBlendFunc = ( stateSet->getAttribute( osg::StateAttribute::BLENDFUNC ) != 0 );
    bool hasTransparentRenderingHint = stateSet->getRenderingHint() == osg::StateSet::TRANSPARENT_BIN;
    bool hasDepthSortBin = ( stateSet->getRenderBinMode() == osg::StateSet::USE_RENDERBIN_DETAILS ) ? 
        ( stateSet->getBinName()=="DepthSortedBin" ) : false;

    // search for the existence of any texture object attributes
    for( unsigned int i=0;i<stateSet->getTextureAttributeList().size();++i )
    {
        const osg::Texture* texture = dynamic_cast< const osg::Texture* >(
            stateSet->getTextureAttribute( i, osg::StateAttribute::TEXTURE ) );
        if( texture != NULL )
        {
            for( unsigned int im=0;im<texture->getNumImages();++im )
            {
                const osg::Image* image = texture->getImage(im);
                if (image && image->isImageTranslucent())
                {
                    hasTranslucentTexture = true;   
                }
            }
        }
    }
    
    return( blendEnabled &&
        ( hasTranslucentTexture || hasBlendFunc || hasTransparentRenderingHint || hasDepthSortBin ) );
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
