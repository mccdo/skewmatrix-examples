// Copyright (c) 2009 Skew Matrix Software LLC. All rights reserved.

#include <osgViewer/Viewer>
#include <osgDB/FileUtils>

#include <osg/Geometry>
#include <osg/Uniform>
#include <osgText/Text>



osg::Node*
createScene()
{
    osg::ref_ptr< osg::Geode > geode = new osg::Geode;

    osg::Geometry* qGeom = osg::createTexturedQuadGeometry( osg::Vec3( -2., 0., -2. ),
        osg::Vec3( 4., 0., 0. ), osg::Vec3( 0., 0., 4. ) );
    osg::ref_ptr< osg::Uniform > textUniform =
        new osg::Uniform( "doText", 0 );
    qGeom->getOrCreateStateSet()->addUniform( textUniform.get() );
    geode->addDrawable( qGeom );

    osgText::Text* t0 = new osgText::Text;
    t0->setText( "Testing" );
    t0->setFont( "arial.ttf" );
    t0->setAxisAlignment( osgText::TextBase::XZ_PLANE );
    t0->setCharacterSize( 1.2 );
    t0->setPosition( osg::Vec3( -1.8, -1., 1. ) );
    t0->setColor( osg::Vec4( 1., 0., 0., 1. ) );
    geode->addDrawable( t0 );

    osgText::Text* t1 = new osgText::Text( *t0 );
    t1->setPosition( osg::Vec3( -1.8, -1., 0. ) );
    t1->setColor( osg::Vec4( 1., 0., 0., .66 ) );
    geode->addDrawable( t1 );

    t1 = new osgText::Text( *t0 );
    t1->setPosition( osg::Vec3( -1.8, -1., -1. ) );
    t1->setColor( osg::Vec4( 1., 0., 0., .33 ) );
    geode->addDrawable( t1 );

    t1 = new osgText::Text( *t0 );
    t1->setPosition( osg::Vec3( -1.8, -1., -2. ) );
    t1->setColor( osg::Vec4( 1., 0., 0., 0. ) );
    geode->addDrawable( t1 );

    osg::StateSet* ss = geode->getOrCreateStateSet();

    textUniform = new osg::Uniform( "doText", 1 );
    ss->addUniform( textUniform.get() );

    osg::ref_ptr< osg::Shader > shader = osg::Shader::readShaderFile(
        osg::Shader::FRAGMENT, osgDB::findDataFile( "shadertext.fs" ) );

    osg::ref_ptr< osg::Program > program = new osg::Program();
    program->addShader( shader.get() );
    ss->setAttribute( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

    osg::ref_ptr< osg::Uniform > textTextureUniform =
        new osg::Uniform( "textTexture", 0 );
    ss->addUniform( textTextureUniform.get() );

    return( geode.release() );
}

int
main( int argc,
      char ** argv )
{
    osg::ref_ptr< osg::Node > root = createScene();

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 600 );
    viewer.getCamera()->setClearColor( osg::Vec4(0,0,0,0) );
    viewer.setSceneData( root.get() );

    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.realize();

    return( viewer.run() );
}

