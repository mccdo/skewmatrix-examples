// Copyright (c) 2013 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osg/ClipNode>
#include <osg/LightModel>
#include <osg/Material>

#include <osgwTools/Shapes.h>
#include <osgwTools/TangentSpaceOp.h>
#include <osgwTools/GeometryModifier.h>


osg::Node* createDefaultScene()
{
    osg::ref_ptr< osg::Group > grp( new osg::Group() );
    osg::ref_ptr< osg::Geode > geode( new osg::Geode() );
    geode->addDrawable( osgwTools::makeBox( osg::Vec3d(1,1,1) ) );
    grp->addChild( geode.get() );
    return( grp.release() );
}

void addClipSubgraph( osg::Group* parent )
{
    const unsigned int planeNumber( 0 );
    osg::ClipNode* cn( new osg::ClipNode() );
    osg::Vec3 n( .9, .5, 0. ); n.normalize();
    cn->addClipPlane( new osg::ClipPlane( planeNumber, n[0], n[1], n[2], 0. ) );
    parent->addChild( cn );
    parent->getOrCreateStateSet()->setMode( GL_CLIP_PLANE0+planeNumber, osg::StateAttribute::ON );
}


void enableTwoSided( osg::Node* node, const bool twoSided )
{
    osg::StateSet* stateSet( node->getOrCreateStateSet() );

    // Enable or disable FFP 2-sided lighting.
    // Has no effect when using shaders.
    osg::ref_ptr< osg::LightModel > lm( new osg::LightModel() );
    lm->setTwoSided( twoSided );
    stateSet->setAttributeAndModes( lm.get() );

    // GLSL 1.20 doesn't have a built-in uniform indicating 2-sided lighting is enabled.
    // We make our own uniform here.
    // Has no effect when using FFP.
    osg::ref_ptr< osg::Uniform > twoSidedUniform( new osg::Uniform( "twoSided", twoSided ) );
    stateSet->addUniform( twoSidedUniform.get() );

    // When on, the hardware will take the two output built-in vertex shader varyings,
    // gl_FrontColor and gl_BackColor, and pass the appropriate one to the fragment shader
    // as the built-in input varying gl_Color.
    // Has no effect when using FFP.
    stateSet->setMode( GL_VERTEX_PROGRAM_TWO_SIDE,
        twoSided ? osg::StateAttribute::ON : osg::StateAttribute::OFF );

    // Non-default material so we can test specular lighting.
    // Used by both FFP and shader rendering.
    osg::ref_ptr< osg::Material > mat( new osg::Material );
    mat->setAmbient( osg::Material::FRONT_AND_BACK, osg::Vec4( 1., 1., 1., 1. ) );
    mat->setDiffuse( osg::Material::FRONT_AND_BACK, osg::Vec4( .2, .6, 1., 1. ) );
    mat->setSpecular( osg::Material::FRONT_AND_BACK, osg::Vec4( 1., 1., 1., 1. ) );
    mat->setShininess( osg::Material::FRONT_AND_BACK, 32. );
    stateSet->setAttributeAndModes( mat.get(), osg::StateAttribute::ON );
}
void setShaders( osg::Node* node, const std::string& vertName, const std::string& fragName, const bool ts=false )
{
    const std::string fullVertName( osgDB::findDataFile( vertName ) );
    const std::string fullFragName( osgDB::findDataFile( fragName ) );
    if( fullVertName.empty() || fullFragName.empty() )
    {
        OSG_FATAL << "Can't find " << vertName << " or " << fragName << std::endl;
        exit( 1 );
    }

    // For fragment shader lighting, use the same vertex and fragment shaders
    // regardless of whether we're lighting in eye coords or tangent space.
    // (Tangent space lighting would be used for bump mapping.)
    // Make the code light in tangent space with a #define.
    const std::string shaderPreamble( "#version 120\n" +
        std::string( ts ? "#define USE_TANGENT_SPACE\n" : "" ) );

    osg::ref_ptr< osg::Shader > vert( new osg::Shader( osg::Shader::VERTEX ) );
    vert->setName( vertName );
    vert->loadShaderSourceFromFile( fullVertName );
    vert->setShaderSource( shaderPreamble + vert->getShaderSource() );

    osg::ref_ptr< osg::Shader > frag( new osg::Shader( osg::Shader::FRAGMENT ) );
    frag->setName( fragName );
    frag->loadShaderSourceFromFile( fullFragName );
    frag->setShaderSource( shaderPreamble + frag->getShaderSource() );

    osg::ref_ptr< osg::Program > prog( new osg::Program );
    prog->addShader( vert.get() );
    prog->addShader( frag.get() );
    if( ts )
    {
        // Program needs explicit attribute location bindings.
        prog->addBindAttribLocation( "a_tangent", 6 );
        prog->addBindAttribLocation( "a_binormal", 7 );
        prog->addBindAttribLocation( "a_normal", 15 );
    }

    osg::StateSet* stateSet( node->getOrCreateStateSet() );
    stateSet->setAttribute( prog.get() );
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    OSG_ALWAYS << "-clip\tAdd clip plane to show model interior." << std::endl;
    const bool clip( arguments.find( "-clip" ) > 0 );

    OSG_ALWAYS << "-ffp\tRender using FFP (default)." << std::endl;
    OSG_ALWAYS << "-vert\tLight in vertex shader." << std::endl;
    OSG_ALWAYS << "-frag\tLight in fragment shader using eye coords." << std::endl;
    OSG_ALWAYS << "-ts\tLight in fragment shader using tangent space." << std::endl;
    const bool vert( arguments.find( "-vert" ) > 0 );
    const bool frag( arguments.find( "-frag" ) > 0 );
    const bool ts( arguments.find( "-ts" ) > 0 );

    OSG_ALWAYS << "-1\tSingle-sided lighting (default)." << std::endl;
    OSG_ALWAYS << "-2\tTwo-sided lighting." << std::endl;
    const bool twoSided( arguments.find( "-2" ) > 0 );

    // Load models or create default scene.
    osg::ref_ptr< osg::Node > scene( osgDB::readNodeFiles( arguments ) );
    if( scene == NULL )
        scene = createDefaultScene();

    // Create scene graph with optional clip node subgraph to show model interior.
    osg::ref_ptr< osg::Group > root( new osg::Group() );
    root->addChild( scene.get() );
    if( clip )
        addClipSubgraph( root.get() );

    // Set state set for single- or two-sided lighting.
    enableTwoSided( root.get(), twoSided );
    if( vert )
    {
        // Per-vertex lighting using shaders.
        setShaders( root.get(), "lightingVert.vert", "lightingVert.frag" );
    }
    else if( frag || ts )
    {
        // Per-fragment lighting using shaders.
        // Lighting is either in eye coords (ts=falst) or tangent space (ts=true).
        setShaders( root.get(), "lightingFrag.vert", "lightingFrag.frag", ts );
        if( ts )
        {
            // Generate tangent and binormal vectors. This requires texture
            // coordinates in unit 0.
            osgwTools::TangentSpaceOp* op = new osgwTools::TangentSpaceOp;
            osgwTools::GeometryModifier gm( op );
            root->accept( gm );
        }
    }


    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return( viewer.run() );
}
