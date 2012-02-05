// Copyright (c) 2012 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osg/Uniform>
#include <osg/Material>


void setRootStateSet( osg::Node* root )
{
    osg::ref_ptr< osg::StateSet > stateSet = root->getOrCreateStateSet();

    // Probably don't need this when we switch to shaders.
    stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    // Add a default Material. Shaders require shininess and specular color.
    osg::ref_ptr< osg::Material > mat = new osg::Material;
    stateSet->setAttribute( mat.get() );

    osg::ref_ptr< osg::Uniform > u = new osg::Uniform( "diffuseMap", 0 );
    stateSet->addUniform( u.get() );
    u = new osg::Uniform( "shadowMap", 1 );
    stateSet->addUniform( u.get() );
    u = new osg::Uniform( "bumpMap", 2 );
    stateSet->addUniform( u.get() );
    u = new osg::Uniform( "normalMap", 3 );
    stateSet->addUniform( u.get() );

    osg::ref_ptr< osg::Program > prog = new osg::Program;

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
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    osg::ref_ptr< osgDB::ReaderWriter::Options > options = new osgDB::ReaderWriter::Options;
    options->setOptionString( "dds_flip" );

    osg::ref_ptr< osg::Node > root = osgDB::readNodeFiles( arguments, options.get() );
    if( !( root.valid() ) )
    {
        osg::notify( osg::FATAL ) << "Can't open model file(s)." << std::endl;
        return( 1 );
    }
    setRootStateSet( root.get() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 450 );
    viewer.setSceneData( root.get() );

    return( viewer.run() );
}
