/* OpenSceneGraph example, osgcomplexsurfshaders.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

/* file:        examples/osgshaders/GL2Scene.cpp
 * author:        Mike Weiblen 2005-05-01
 *
 * Compose a scene of several instances of a model, with a different
 * OpenGL Shading Language shader applied to each.
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
*/

// Complex Surfaces example built from osgshaders Sept 2010 by Chris 'Xenon' Hanson, xenon@alphapixel.com

#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Geode>
#include <osg/Node>
#include <osg/Material>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgUtil/Optimizer>

#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>

#include <iostream>

#include "GL2Scene.h"
#include "Noise.h"

//#define CONCRETE
//#define GRASS
#define DIRT


static osg::ref_ptr<osg::Group> rootNode;

// Create some geometry upon which to render GLSL shaders.
static osg::Geode*
CreateModel()
{
    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f,0.0f,0.0f),0.5f,0.5f,0.01f)));
    return geode;
}

// Add a reference to the masterModel at the specified translation, and
// return its StateSet so we can easily attach StateAttributes.
static osg::StateSet*
ModelInstance()
{
    static float xvalue = -1.5f;
    static osg::Node* masterModel = CreateModel();

    osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform();
    xform->setPosition(osg::Vec3( xvalue, 0.0f, 0.0f ));
    xvalue += 1.5f;
    xform->addChild(masterModel);
    rootNode->addChild(xform);
    return xform->getOrCreateStateSet();
}

// load source from a file.
static void
LoadShaderSource( osg::Shader* shader, const std::string& fileName )
{
    std::string fqFileName = osgDB::findDataFile(fileName);
    if( fqFileName.length() != 0 )
    {
        shader->loadShaderSourceFromFile( fqFileName.c_str() );
    }
    else
    {
        osg::notify(osg::WARN) << "File \"" << fileName << "\" not found." << std::endl;
    }
}


///////////////////////////////////////////////////////////////////////////
// rude but convenient globals

static osg::Program* ConcreteProgram;
static osg::Shader*  ConcreteVertObj;
static osg::Shader*  ConcreteFragObj;

static osg::Program* GrassProgram;
static osg::Shader*  GrassVertObj;
static osg::Shader*  GrassFragObj;

static osg::Program* DirtProgram;
static osg::Shader*  DirtVertObj;
static osg::Shader*  DirtFragObj;



///////////////////////////////////////////////////////////////////////////
// Compose a scenegraph with examples of GLSL shaders

#define TEXUNIT_PERM        1
#define TEXUNIT_BUMP        2
#define TEXUNIT_DARK        3
#define TEXUNIT_GRASS       4

osg::ref_ptr<osg::Group>
GL2Scene::buildScene()
{
    //osg::Texture3D* noiseTexture = make3DNoiseTexture( 32 /*128*/ );
    //osg::Texture1D* sineTexture = make1DSineTexture( 32 /*1024*/ );

    // the root of our scenegraph.
    rootNode = new osg::Group;

    osg::Image* PermImage   = osgDB::readImageFile("images/permTexture.png");
	osg::Texture2D* PermTexture = new osg::Texture2D(PermImage);
    PermTexture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
    PermTexture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );

	// attach some Uniforms to the root, to be inherited by Programs.
    {
        //osg::Uniform* OffsetUniform = new osg::Uniform( "Offset", osg::Vec3(0.0f, 0.0f, 0.0f) );

        //osg::StateSet* ss = rootNode->getOrCreateStateSet();
        //ss->addUniform( OffsetUniform );
    }

    // Concrete Shader
#if defined( CONCRETE )
    {
	    osg::Image* ConcreteDarkenImage = osgDB::readImageFile("images/ConcreteDarken.png");
	    osg::Image* ConcreteBumpImage   = osgDB::readImageFile("images/ConcreteBump.png");
		osg::Texture2D* DarkenTexture = new osg::Texture2D(ConcreteDarkenImage);
        DarkenTexture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
        DarkenTexture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
		osg::Texture2D* BumpTexture = new osg::Texture2D(ConcreteBumpImage);
        BumpTexture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT );
        BumpTexture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT );
        osg::StateSet* ss = ModelInstance();

		ss->setTextureAttribute(TEXUNIT_DARK, DarkenTexture);
		ss->setTextureAttribute(TEXUNIT_BUMP, BumpTexture);
		ss->setTextureAttribute(TEXUNIT_PERM, PermTexture);

        ss->addUniform( new osg::Uniform("baseMap", TEXUNIT_DARK) );
        ss->addUniform( new osg::Uniform("bumpMap", TEXUNIT_BUMP) );
        ss->addUniform( new osg::Uniform("permTexture", TEXUNIT_PERM) );

        ss->addUniform( new osg::Uniform("fSpecularPower", 25.0f) );
        ss->addUniform( new osg::Uniform("fScale", 4.0f) );
		
		ss->addUniform( new osg::Uniform("fvLightPosition", osg::Vec3(-100.0f, 0.0f, 100.0f)) );
		ss->addUniform( new osg::Uniform("fvEyePosition", osg::Vec3(0.0f, 0.0f, 100.0f)) );

		ss->addUniform( new osg::Uniform("fvDiffuse", osg::Vec4(0.8863f, 0.8850f, 0.8850f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvSpecular", osg::Vec4(0.0157f, 0.0157f, 0.0157f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvAmbient", osg::Vec4(0.3686f, 0.3686f, 0.3686f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvBaseColorA", osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvBaseColorB", osg::Vec4(0.9882f, 0.9579f, 0.9158f, 1.0f)) );

		ConcreteProgram = new osg::Program;
        ConcreteProgram->setName( "Concrete" );
        _programList.push_back( ConcreteProgram );
        ConcreteVertObj = new osg::Shader( osg::Shader::VERTEX );
        ConcreteFragObj = new osg::Shader( osg::Shader::FRAGMENT );
        ConcreteProgram->addShader( ConcreteFragObj );
        ConcreteProgram->addShader( ConcreteVertObj );
        ss->setAttributeAndModes(ConcreteProgram, osg::StateAttribute::ON);
    }
#elif defined( GRASS )
	// Grass Shader
	{
        osg::StateSet* ss = ModelInstance();

		ss->setTextureAttribute(TEXUNIT_PERM, PermTexture);

        ss->addUniform( new osg::Uniform("permTexture", TEXUNIT_PERM) );

        ss->addUniform( new osg::Uniform("fSpecularPower", 25.0f) );
        ss->addUniform( new osg::Uniform("fScale", 1.403f) );
        ss->addUniform( new osg::Uniform("fMacroToMicroScale", -0.56f) );
		
		ss->addUniform( new osg::Uniform("fvLightPosition", osg::Vec3(-100.0f, 0.0f, 100.0f)) );
		ss->addUniform( new osg::Uniform("fvEyePosition", osg::Vec3(0.0f, 0.0f, 100.0f)) );

		ss->addUniform( new osg::Uniform("fvDiffuse", osg::Vec4(0.8863f, 0.8850f, 0.8850f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvSpecular", osg::Vec4(0.0157f, 0.0157f, 0.0157f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvAmbient", osg::Vec4(0.3686f, 0.3686f, 0.3686f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvBaseColorA", osg::Vec4(0.1098f, 0.6692f, 0.1628f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvBaseColorB", osg::Vec4(0.0863f, 0.5564f, 0.1156f, 1.0f)) );

		GrassProgram = new osg::Program;
        GrassProgram->setName( "Grass" );
        _programList.push_back( GrassProgram );
        GrassVertObj = new osg::Shader( osg::Shader::VERTEX );
        GrassFragObj = new osg::Shader( osg::Shader::FRAGMENT );
        GrassProgram->addShader( GrassFragObj );
        GrassProgram->addShader( GrassVertObj );
        ss->setAttributeAndModes(GrassProgram, osg::StateAttribute::ON);
    }
#else
	// Dirt Shader
    {
        osg::StateSet* ss = ModelInstance();

		ss->setTextureAttribute(TEXUNIT_PERM, PermTexture);

        ss->addUniform( new osg::Uniform("permTexture", TEXUNIT_PERM) );

        ss->addUniform( new osg::Uniform("fSpecularPower", 25.0f) );
        ss->addUniform( new osg::Uniform("fScale", 1.403f) );
        ss->addUniform( new osg::Uniform("fGravelScale", 8.0f) );
        ss->addUniform( new osg::Uniform("fDirtScale", 0.15f) );
        ss->addUniform( new osg::Uniform("fDirtGravelDistScale", 0.06f) );
		
		ss->addUniform( new osg::Uniform("fvLightPosition", osg::Vec3(-100.0f, 0.0f, 100.0f)) );
		ss->addUniform( new osg::Uniform("fvEyePosition", osg::Vec3(0.0f, 0.0f, 100.0f)) );

		ss->addUniform( new osg::Uniform("fvDiffuse", osg::Vec4(0.8863f, 0.8850f, 0.8850f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvSpecular", osg::Vec4(0.0157f, 0.0157f, 0.0157f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvAmbient", osg::Vec4(0.3686f, 0.3686f, 0.3686f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvBaseColorA", osg::Vec4(0.7255f, 0.6471f, 0.3451f, 1.0f)) );
		ss->addUniform( new osg::Uniform("fvBaseColorB", osg::Vec4(0.6392f, 0.5412f, 0.2118f, 1.0f)) );

		DirtProgram = new osg::Program;
        DirtProgram->setName( "Dirt" );
        _programList.push_back( DirtProgram );
        DirtVertObj = new osg::Shader( osg::Shader::VERTEX );
        DirtFragObj = new osg::Shader( osg::Shader::FRAGMENT );
        DirtProgram->addShader( DirtFragObj );
        DirtProgram->addShader( DirtVertObj );
        ss->setAttributeAndModes(DirtProgram, osg::StateAttribute::ON);
    }
#endif

    reloadShaderSource();

    return rootNode;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

GL2Scene::GL2Scene()
{
    _rootNode = buildScene();
    _shadersEnabled = true;
}

GL2Scene::~GL2Scene()
{
}

void
GL2Scene::reloadShaderSource()
{
    osg::notify(osg::INFO) << "reloadShaderSource()" << std::endl;

#if defined( CONCRETE )
    LoadShaderSource( ConcreteVertObj, "shaders/basicbumpmap.vert" );
    LoadShaderSource( ConcreteFragObj, "shaders/concrete.frag" );
#elif defined( GRASS )
    LoadShaderSource( GrassVertObj, "shaders/basicbumpmap.vert" );
    LoadShaderSource( GrassFragObj, "shaders/grass.frag" );
#else
    LoadShaderSource( DirtVertObj, "shaders/basicbumpmap.vert" );
    LoadShaderSource( DirtFragObj, "shaders/dirt.frag" );
#endif
}



/*EOF*/
