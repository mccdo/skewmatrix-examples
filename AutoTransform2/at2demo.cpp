// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

// at2demo.cpp - demo of shader-based AutoTransform2
// mike.weiblen@gmail.com
// 2010-08-09

//#define FILENAME        "at2test.osg"
#define FILENAME        "cow.osg"


#include <osg/Geode>
#include <osg/Node>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/Group>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>
#include <osg/BoundingSphere>

#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgViewer/Viewer>

#include <iostream>
#include <string>


///////////////////////////////////////////////////////////////////////////

static void loadShaderSource( osg::Shader* shader )
{
    std::string fileName( shader->getName() );
    std::string fqFileName( osgDB::findDataFile(fileName) );
    if( fqFileName.empty() )
    {
        osg::notify(osg::WARN) << "File \"" << fileName << "\" not found." << std::endl;
        return;
    }

    shader->loadShaderSourceFromFile( fqFileName.c_str() );
}


static osg::Shader* at2FragShader = 0;
static osg::Shader* at2VertShader = 0;


static void reloadShaders()
{
    loadShaderSource( at2FragShader );
    loadShaderSource( at2VertShader );
}


static void addAutoTransform2( osg::Node* node )
{
    at2VertShader = new osg::Shader( osg::Shader::VERTEX );
    at2VertShader->setName( "AutoTransform2.vs" );
    at2FragShader = new osg::Shader( osg::Shader::FRAGMENT );
    at2FragShader->setName( "AutoTransform2.fs" );
    reloadShaders();

    osg::Program* program = new osg::Program;
    program->setName( "AutoTransform2" );
    program->addShader( at2FragShader );
    program->addShader( at2VertShader );
    osg::StateSet* ss = node->getOrCreateStateSet();
    ss->setAttributeAndModes( program, osg::StateAttribute::ON );

    // default values for AutoTransform2
    ss->addUniform( new osg::Uniform( "at2_PivotPoint", osg::Vec3() ) );
    ss->addUniform( new osg::Uniform( "at2_Scale", 1.0f ) );
}


/////////////////////////////////////////////////////////////////////////////

osg::Node* createScene()
{
    // load a model to test AutoTransform2
    osg::Node* model( osgDB::readNodeFile( FILENAME ) );
    if( ! model )
    {
        osg::notify( osg::FATAL ) << "cant load " FILENAME << std::endl;
        exit(1);
    }

    // use the model's bounding sphere to defined points-of-interest
    const osg::BoundingSphere bsphere( model->getBound() );
    osg::Vec3 at2PivotPoint( bsphere._center );
    float at2Scale( 1.0f );

    // override the AT2's default values for this model.
    osg::StateSet* ss = model->getOrCreateStateSet();
    ss->addUniform( new osg::Uniform( "at2_PivotPoint", at2PivotPoint ) );
    ss->addUniform( new osg::Uniform( "at2_Scale", at2Scale ) );

    // add two models: one under the AT2, the other as control under the root
    osg::Group* root( new osg::Group );
    root->addChild( model );

    osg::Group* at2( new osg::Group );
    addAutoTransform2( at2 );
    root->addChild( at2 );
    at2->addChild( model );

    return root;
}

/////////////////////////////////////////////////////////////////////////////

class KeyHandler: public osgGA::GUIEventHandler
{
    public:
        KeyHandler() {}

        bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
        {
            if( ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN )
                return false;

            switch( ea.getKey() )
            {
                case 'x':
                    reloadShaders();
                    return true;
            }
            return false;
        }
};

/////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
    osgViewer::Viewer viewer;
    viewer.setSceneData( createScene() );
    viewer.addEventHandler( new KeyHandler() );
    return viewer.run();
}

// vim: set sw=4 ts=8 et ic ai:
