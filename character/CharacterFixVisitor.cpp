// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include "CharacterFixVisitor.h"

#include <osgDB/FileNameUtils>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/Image>
#include <osg/Material>
#include <osg/Notify>
#include <osgwTools/InsertRemove.h>


CharacterFixVisitor::CharacterFixVisitor( osg::NodeVisitor::TraversalMode mode )
  : osg::NodeVisitor( mode ),
    _scale( 0.03280839895013123359 ), // convert cm to feet
    _reverseNormals( true ),
    _stripTexturePaths( true ),
    _texturePrefix( std::string( "" ) )
{
}

osg::Node*
CharacterFixVisitor::process( osg::Node& node )
{
    node.accept( *this );

    osg::ref_ptr< osg::MatrixTransform > mt = new osg::MatrixTransform(
        osg::Matrix::scale( _scale, _scale, _scale ) );
    mt->addChild( &node );
    return( mt.release() );
}

// Set/get the vertex scaling.
void
CharacterFixVisitor::setScaleFactor( double scale )
{
    _scale = scale;
}
double
CharacterFixVisitor::getScaleFactor() const
{
    return( _scale );
}
// Set/get whether or not normals should be flipped, or
// reversed, or nagated. The .fbx models seem to have them
// pointing inwards for some reason.
void
CharacterFixVisitor::setReverseNormals( bool reverse )
{
    _reverseNormals = reverse;
}
bool
CharacterFixVisitor::getReverseNormals() const
{
    return( _reverseNormals );
}
// Set how to handle texture paths. The paths can be stripped from
// the names by setting strip=true. When stripping is on, the
// prefex can be prepended to the bare file name.
void
CharacterFixVisitor::setTexturePathControl( bool strip, const std::string& prefix )
{
    _stripTexturePaths = strip;
    _texturePrefix = prefix;
}


void
CharacterFixVisitor::apply( osg::Group& node )
{
    osg::notify( osg::DEBUG_INFO ) << "Group" << std::endl;

    // Process the StateSet
    if( node.getStateSet() != NULL )
        applyStateSet( node.getStateSet() );

    // Traverse
    preTraverse( node );
    traverse( node );
    postTraverse( node );
}

void
CharacterFixVisitor::apply( osg::Node& node )
{
    // Something other than a Group or a Geode. This is unusual.
    osg::notify( osg::WARN ) << "CharacterFixVisitor: Unsupported Node: " <<
        node.className() << std::endl;

    // Process the StateSet
    if( node.getStateSet() != NULL )
        applyStateSet( node.getStateSet() );

    // no-op.
    traverse( node );
}

void
CharacterFixVisitor::apply( osg::Geode& node )
{
    osg::notify( osg::DEBUG_INFO ) << "Geode" << std::endl;

    // Process the StateSet
    if( node.getStateSet() != NULL )
        applyStateSet( node.getStateSet() );

    // Loop over Drawables and process any Geometry objects that we find.
    unsigned int idx;
    for( idx=0; idx<node.getNumDrawables(); idx++ )
    {
        osg::Drawable* draw = node.getDrawable( idx );
        osg::Geometry* geom = draw->asGeometry();
        if( geom != NULL )
            applyGeometry( geom );
    }

    // no-op.
    traverse( node );
}



// Before traversing further, remove children that we
// know we don't want, such as LightSource and CameraView.
void
CharacterFixVisitor::preTraverse( osg::Group& grp )
{
    osg::NodeList removeList;
    unsigned int idx;
    for( idx=0; idx<grp.getNumChildren(); idx++ )
    {
        osg::Node* node = grp.getChild( idx );
        if( node->className() == std::string( "LightSource" ) )
            removeList.push_back( node );
        if( node->className() == std::string( "CameraView" ) )
            removeList.push_back( node );
    }

    osg::NodeList::iterator it;
    for( it=removeList.begin(); it != removeList.end(); it++ )
    {
        bool result = grp.removeChild( (*it).get() );
        osg::notify( osg::DEBUG_INFO ) << "Pre-traverse name: " << (*it)->getName() <<
            ", class: " << (*it)->className() << std::endl;
    }
}

// After traversing, remove all children that meet the following
// conditions: They are Groups, they have no children, and they
// are not osgAnimation::Bone objects.
void
CharacterFixVisitor::postTraverse( osg::Group& grp )
{
    osg::NodeList removeList;
    unsigned int idx;
    for( idx=0; idx<grp.getNumChildren(); idx++ )
    {
        osg::Node* cNode = grp.getChild( idx );
        osg::Group* cGrp = cNode->asGroup();
        if( ( cGrp != NULL ) &&
            ( cGrp->getNumChildren() == 0 ) &&
            (cGrp->className() != std::string( "Bone" ) ) )
            removeList.push_back( cGrp );
    }

    osg::NodeList::iterator it;
    for( it=removeList.begin(); it != removeList.end(); it++ )
    {
        bool result = grp.removeChild( (*it).get() );
        osg::notify( osg::DEBUG_INFO ) << "Post-traverse name: " << (*it)->getName() <<
            ", class: " << (*it)->className() << std::endl;
    }
}

// Handle texture path control based on the strip and prefix settings.
// Set a default material. Enable normal rescaling.
void
CharacterFixVisitor::applyStateSet( osg::StateSet* ss )
{
    osg::notify( osg::DEBUG_INFO ) << "In applyStateSet" << std::endl;

    if( _stripTexturePaths )
    {
        osg::StateAttribute* sa( ss->getTextureAttribute(
            0, osg::StateAttribute::TEXTURE ) );

        osg::Texture* tex( NULL );
        if( sa != NULL )
            tex = sa->asTexture();

        osg::Image* image( NULL );
        if( tex != NULL )
            image = tex->getImage( 0 );

        if( image != NULL )
        {
            std::string texName( image->getFileName() );
            osg::notify( osg::DEBUG_INFO ) << "Input texture name: " << texName << std::endl;

            const std::string outName( _texturePrefix + osgDB::getSimpleFileName( texName ) );
            osg::notify( osg::DEBUG_INFO ) << "  Output texture name: " << outName << std::endl;

            image->setFileName( outName );
        }
    }

    osg::Material* mat = new osg::Material;
    mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( .3f, .3f, .3f, 1. ) );
    mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( 1., 1., 1., 1. ) );
    mat->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4( 0., 0., 0., 1. ) );
    ss->setAttributeAndModes( mat, osg::StateAttribute::ON );

    ss->setMode( GL_RESCALE_NORMAL, osg::StateAttribute::ON );

    // This was used during debug. No longer needed.
    //ss->setMode( GL_CULL_FACE, osg::StateAttribute::ON );
    //ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    //ss->setTextureMode( 0, GL_TEXTURE_2D, osg::StateAttribute::OFF );
}

// For normals, flip them if requested to do so.
void
CharacterFixVisitor::applyGeometry( osg::Geometry* geom )
{
    osg::notify( osg::DEBUG_INFO ) << "In applyGeometry" << std::endl;

    if( geom->getStateSet() != NULL )
        applyStateSet( geom->getStateSet() );

    osg::Vec3Array* normals = dynamic_cast< osg::Vec3Array* >(
        geom->getNormalArray() );
    if( ( normals != NULL ) && _reverseNormals )
    {
        osg::notify( osg::DEBUG_INFO ) << "Reversing normals: " << normals->size() << std::endl;
        osg::Vec3Array::iterator it;
        for( it = normals->begin(); it != normals->end(); it++ )
        {
            osg::Vec3& n = *it;
            n = -n;
        }
    }
}
