// Copyright (c) 2013 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osg/ClipNode>
#include <osg/LightModel>

#include <osgwTools/Shapes.h>


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
}
void setShaders( osg::Node* node, const std::string& vertName, const std::string& fragName )
{
    const std::string fullVertName( osgDB::findDataFile( vertName ) );
    const std::string fullFragName( osgDB::findDataFile( fragName ) );
    if( fullVertName.empty() || fullFragName.empty() )
    {
        OSG_FATAL << "Can't find " << vertName << " or " << fragName << std::endl;
        exit( 1 );
    }

    osg::ref_ptr< osg::Shader > vert( new osg::Shader( osg::Shader::VERTEX ) );
    vert->loadShaderSourceFromFile( fullVertName );

    osg::ref_ptr< osg::Shader > frag( new osg::Shader( osg::Shader::FRAGMENT ) );
    frag->loadShaderSourceFromFile( fullFragName );

    osg::ref_ptr< osg::Program > prog( new osg::Program );
    prog->addShader( vert.get() );
    prog->addShader( frag.get() );

    osg::StateSet* stateSet( node->getOrCreateStateSet() );
    stateSet->setAttributeAndModes( prog.get() );

}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    OSG_ALWAYS << "-clip\tAdd clip plane." << std::endl;
    const bool clip( arguments.find( "-clip" ) > 0 );

    OSG_ALWAYS << "-ffp\tRender using FFP (default)." << std::endl;
    OSG_ALWAYS << "-vert\tLight in vertex shader." << std::endl;
    OSG_ALWAYS << "-frag\tLight in fragment shader." << std::endl;
    const bool vert( arguments.find( "-vert" ) > 0 );
    const bool frag( arguments.find( "-frag" ) > 0 );

    OSG_ALWAYS << "-1\tSingle-sided lighting (default)." << std::endl;
    OSG_ALWAYS << "-2\tTwo-sided lighting." << std::endl;
    const bool twoSided( arguments.find( "-2" ) > 0 );

    osg::ref_ptr< osg::Node > scene( osgDB::readNodeFiles( arguments ) );
    if( scene == NULL )
        scene = createDefaultScene();

    osg::ref_ptr< osg::Group > root( new osg::Group() );
    root->addChild( scene.get() );
    if( clip )
        addClipSubgraph( root.get() );

    enableTwoSided( root.get(), twoSided );
    if( vert )
        setShaders( root.get(), "lightingVert.vert", "lightingVert.frag" );
    else if( frag )
        setShaders( root.get(), "lightingFrag.vert", "lightingFrag.frag" );


    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return( viewer.run() );
}
