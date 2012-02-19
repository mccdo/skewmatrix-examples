// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#include "RenderPrep.h"
#include <osgwTools/StateTrackingNodeVisitor.h>

#include <osgwTools/GeometryModifier.h>
#include <osgwTools/TangentSpaceOp.h>

#include <osgDB/FileUtils>
#include <osg/Uniform>
#include <osg/Material>
#include <osgText/Text>


RenderPrep::RenderPrep( osg::Node* root, const float textSize, bool noPM )
  : osgwTools::StateTrackingNodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
    _textSize( textSize )
{
    //
    // Init internal variables.
    _isOsgTextUniform = new osg::Uniform( "isOsgText", true );



    //
    // Debugging. Add some osgText.
    if( _textSize > 0. )
    {
        osg::Geode* geode = new osg::Geode;
        osgText::Text* text = new osgText::Text;
        geode->addDrawable( text );
        root->asGroup()->addChild( geode );

        text->setPosition( osg::Vec3( 0., 0., 260. ) );
        text->setFont( "arial.ttf" );
        text->setText( "Tony Rawkz!" );
        text->setColor( osg::Vec4( 1.f, .6f, 0.f, 1.0f ) );
        text->setAlignment( osgText::Text::LEFT_BOTTOM );
        text->setAxisAlignment( osgText::Text::XZ_PLANE );
        text->setCharacterSize( _textSize );
    }



    //
    // Prep the top of the scene graph.

    osg::ref_ptr< osg::StateSet > stateSet = root->getOrCreateStateSet();

    // Add a default Material. Shaders require shininess and specular color.
    osg::ref_ptr< osg::Material > mat = new osg::Material;
    stateSet->setAttribute( mat.get() );

    osg::ref_ptr< osg::Uniform > u = new osg::Uniform( "diffuseMap", 2 );
    stateSet->addUniform( u.get() );
    u = new osg::Uniform( "shadowMap", 1 );
    stateSet->addUniform( u.get() );
    u = new osg::Uniform( "normalMap", 0 );
    stateSet->addUniform( u.get() );

    u = new osg::Uniform( "isOsgText", false );
    stateSet->addUniform( u.get() );
    u = new osg::Uniform( "parallaxMap", !noPM );
    stateSet->addUniform( u.get() );

    u = new osg::Uniform( "attenuation", 1500.f );
    stateSet->addUniform( u.get() );

    osg::ref_ptr< osg::Program > prog = new osg::Program;
    prog->addBindAttribLocation( "a_tangent", 6 );
    prog->addBindAttribLocation( "a_binormal", 7 );

    std::string shaderName( "mayaViewer.vs" );
    osg::ref_ptr< osg::Shader > s = new osg::Shader( osg::Shader::VERTEX );
    s->setName( shaderName );
    s->loadShaderSourceFromFile( osgDB::findDataFile( shaderName ) );
    if( s->getShaderSource().empty() )
        osg::notify( osg::WARN ) << "Can't load " << shaderName << std::endl;
    prog->addShader( s.get() );

    shaderName = "mayaViewer.fs";
    s = new osg::Shader( osg::Shader::FRAGMENT );
    s->setName( shaderName );
    s->loadShaderSourceFromFile( osgDB::findDataFile( shaderName ) );
    if( s->getShaderSource().empty() )
        osg::notify( osg::WARN ) << "Can't load " << shaderName << std::endl;
    prog->addShader( s.get() );

    stateSet->setAttribute( prog.get() );

    // Add additional uniforms as needed and do any other per-node prep work.
    root->accept( *this );


    // Generate tangent/binormal vertex attributes.
    osgwTools::TangentSpaceOp* tso = new osgwTools::TangentSpaceOp;
    osgwTools::GeometryModifier gm( tso );
    root->accept( gm );
}
RenderPrep::~RenderPrep()
{
}

void RenderPrep::apply( osg::Node& node )
{
    pushStateSet( node.getStateSet() );

    processStateSet( node );

    traverse( node );
    popStateSet();
}
void RenderPrep::apply( osg::Geode& node )
{
    pushStateSet( node.getStateSet() );

    processStateSet( node );

    unsigned int idx;
    for( idx=0; idx<node.getNumDrawables(); idx++ )
        applyDrawable( node.getDrawable( idx ) );

    traverse( node );
    popStateSet();
}
void RenderPrep::applyDrawable( osg::Drawable* draw )
{
    if( dynamic_cast< osgText::Text* >( draw ) != NULL )
    {
        osg::StateSet* stateSet = draw->getOrCreateStateSet();
        stateSet->addUniform( _isOsgTextUniform.get() );
    }

    pushStateSet( draw->getStateSet() );

    processStateSet( draw );

    popStateSet();
}

void RenderPrep::processStateSet( osg::Node& node )
{
    // Currently a no-op.
}

void RenderPrep::processStateSet( osg::Drawable* draw )
{
    // Currently a no-op.

    // Get top of stack.
    //osg::StateSet* tos = _stateStack.back().get();
}
