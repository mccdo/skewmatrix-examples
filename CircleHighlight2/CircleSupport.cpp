// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include "CircleSupport.h"
#include <osgDB/FileUtils>
#include <osgwTools/AbsoluteModelTransform.h>
#include <osg/Geode>
#include <osgText/Text>


CircleSupport::CircleSupport()
{
}
CircleSupport::~CircleSupport()
{
}


osg::Node*
CircleSupport::createCircleHighlight( const osg::NodePath& nodePath, const osg::Node& pickedNode )
{
    // determine radius 
    osg::BoundingSphere sphere( pickedNode.getBound() );
    const double radius( sphere.radius() );

    // Structure:
    //   AbsoluteModelTransform->CircleGeode-->Circle (Geometry)
    //                                   \---->Line segment (Geometry)
    //                                    \--->Label (osgText::Text)

    osg::Matrix xform = osg::computeLocalToWorld( nodePath ) *
        osg::Matrix::translate( sphere.center() );

    osg::ref_ptr< osgwTools::AbsoluteModelTransform > amt =
        new osgwTools::AbsoluteModelTransform( xform );
    amt->getOrCreateStateSet()->addUniform( new osg::Uniform( "circleRadius", (float)( radius ) ) );

    osg::ref_ptr< osg::Geode > circleGeode = new osg::Geode;
    amt->addChild( circleGeode.get() );

    osg::Drawable* circleInput( createPoint() );
    circleGeode->addDrawable( circleInput );


    if( !_labelText.empty() )
    {
        // Add text annotation

        // Determine text pos and line segment endpoints.
        // In xy (z=0) plane.
        // NOTE: These computations are mirrored in the geometry shader (circle.gs)
        // to draw the tag line from the circle to the text.
        osg::Vec3 textDirection( 1., 1., 0. );
        textDirection.normalize();
        osg::Vec3 textPos( textDirection * radius * 1.4f );

        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setPosition( textPos );
        text->setFont( "arial.ttf" );
        text->setText( _labelText );
        text->setColor( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        text->setAlignment( osgText::Text::LEFT_BOTTOM );
        // In xy (z=0) plans.
        text->setAxisAlignment( osgText::Text::XY_PLANE );

        // This is how we render osgText using shaders.
        // See circleText.fs for fragment shader code.
        text->getOrCreateStateSet()->setAttribute( _textProgram.get() );

        // Character size goes up as a function of distance.
        // Basis: Size is 0.1 for a distance of 10.0.
        // TBD hack, need to auto size the text in the shader.
        float size( 0.01f * 40.f );
        osg::notify( osg::DEBUG_FP ) << "    Using char size " << size << std::endl;
        text->setCharacterSize( size );

        circleGeode->addDrawable( text.get() );
    } // if

    return( amt.release() );
} // createCircleHighlight

void
CircleSupport::createCircleState( osg::StateSet* ss )
{
    // turn off depth testing on our subgraph
    ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );
    ss->setRenderBinDetails( 1000, "RenderBin" );

    // Load shaders for rendering the circle highlight.
    _lineStripProgram = new osg::Program;
    _lineStripProgram->setName( "Circle line strip shader" );

    osg::ref_ptr< osg::Shader > vShader = new osg::Shader( osg::Shader::VERTEX );
    std::string shaderFileName( "circle.vs" );
    shaderFileName = osgDB::findDataFile( shaderFileName );
    if( shaderFileName.empty() )
    {
        osg::notify(osg::WARN) << "File \"" << shaderFileName << "\" not found." << std::endl;
        return;
    }
    vShader->loadShaderSourceFromFile( shaderFileName );
    _lineStripProgram->addShader( vShader.get() );

    osg::ref_ptr< osg::Shader > gShader = new osg::Shader( osg::Shader::GEOMETRY );
    shaderFileName = "circle.gs";
    shaderFileName = osgDB::findDataFile( shaderFileName );
    if( shaderFileName.empty() )
    {
        osg::notify(osg::WARN) << "File \"" << shaderFileName << "\" not found." << std::endl;
        return;
    }
    gShader->loadShaderSourceFromFile( shaderFileName );
    _lineStripProgram->addShader( gShader.get() );

    // If circle approx is 'maxApprox' vertices,
    //   ...add another to close the circle,
    //     ...and add two more for the label text tag.
    const float maxApprox( 40.f );
    _lineStripProgram->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, (int)(maxApprox) + 3 );
    _lineStripProgram->setParameter( GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS );
    _lineStripProgram->setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_LINE_STRIP );

    osg::ref_ptr< osg::Shader > fShader = new osg::Shader( osg::Shader::FRAGMENT );
    shaderFileName = "circle.fs";
    shaderFileName = osgDB::findDataFile( shaderFileName );
    if( shaderFileName.empty() )
    {
        osg::notify(osg::WARN) << "File \"" << shaderFileName << "\" not found." << std::endl;
        return;
    }
    fShader->loadShaderSourceFromFile( shaderFileName );
    _lineStripProgram->addShader( fShader.get() );

    // HACK to work around a bug in OSG. Do not attach the program now, because
    // _labelGroup currently has nothing to draw. Attaching a program to such an
    // "empty" program causes the program to "leak" to other sibling nodes.
    //ss->setAttribute( _lineStripProgram );

    _textProgram = new osg::Program;
    _textProgram->setName( "Circle text label shader" );

    osg::ref_ptr< osg::Shader > tvShader = new osg::Shader( osg::Shader::VERTEX );
    shaderFileName = "circleText.vs";
    shaderFileName = osgDB::findDataFile( shaderFileName );
    if( shaderFileName.empty() )
    {
        osg::notify(osg::WARN) << "File \"" << shaderFileName << "\" not found." << std::endl;
        return;
    }
    tvShader->loadShaderSourceFromFile( shaderFileName );
    _textProgram->addShader( tvShader.get() );

    osg::ref_ptr< osg::Shader > tfShader = new osg::Shader( osg::Shader::FRAGMENT );
    shaderFileName = "circleText.fs";
    shaderFileName = osgDB::findDataFile( shaderFileName );
    if( shaderFileName.empty() )
    {
        osg::notify(osg::WARN) << "File \"" << shaderFileName << "\" not found." << std::endl;
        return;
    }
    tfShader->loadShaderSourceFromFile( shaderFileName );
    _textProgram->addShader( tfShader.get() );


    // default values for AutoTransform2
    ss->addUniform( new osg::Uniform( "at2_PivotPoint", osg::Vec3() ) );
    ss->addUniform( new osg::Uniform( "at2_Scale", 0.0f ) ); // Don't scale

    // Controls for circle geometry shader.
    ss->addUniform( new osg::Uniform( "circleRadius", 1.f ) );
    ss->addUniform( new osg::Uniform( "circleMaxApprox", maxApprox ) );

    // Support for texture mapped text.
    ss->addUniform( new osg::Uniform( "circleTextSampler", 0 ) );
}

osg::Drawable*
CircleSupport::createPoint()
{
    osg::ref_ptr< osg::Geometry > geom = new osg::Geometry;

    osg::Vec3Array* v = new osg::Vec3Array;
    v->push_back( osg::Vec3( 0., 0., 0. ) );
    geom->setVertexArray( v );

    osg::Vec4Array* c = new osg::Vec4Array;
    c->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    geom->setColorArray( c );
    geom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );

    geom->addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, 1 ) );

    return( geom.release() );
}

void
CircleSupport::setLabelText( const std::string& labelText )
{
    _labelText = labelText;
}

const std::string&
CircleSupport::getLabelText() const
{
    return( _labelText );
}



CameraUpdateViewportCallback::CameraUpdateViewportCallback()
{
}
CameraUpdateViewportCallback::~CameraUpdateViewportCallback()
{
}

void
CameraUpdateViewportCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    if( nv->getVisitorType() != osg::NodeVisitor::UPDATE_VISITOR )
    {
        osg::notify( osg::WARN ) << "CameraUpdateViewportCallback is intended to be used as an update callback." << std::endl;
        traverse( node,nv );
        return;
    }
    osg::Camera* cam = dynamic_cast< osg::Camera* >( node );
    if( cam == NULL )
    {
        osg::notify( osg::WARN ) << "CameraUpdateViewportCallback should be attached to the top-level Camera." << std::endl;
        traverse( node,nv );
        return;
    }

    osg::StateSet* ss = node->getOrCreateStateSet();
    osg::Uniform* uniform = ss->getUniform( "viewport" );
    if( uniform == NULL )
    {
        uniform = new osg::Uniform( osg::Uniform::FLOAT_VEC2, "viewport" );
        ss->addUniform( uniform );
    }

    const osg::Viewport* vp = cam->getViewport();
    uniform->set( osg::Vec2f( vp->width(), vp->height() ) );

    traverse(node,nv);
}
