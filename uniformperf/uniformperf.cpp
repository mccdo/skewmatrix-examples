// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Program>
#include <osg/Uniform>
#include <osg/NodeVisitor>
#include <osg/CullFace>

#include <string>
#include <sstream>


static const unsigned int winW( 800 ), winH( 600 );
static const double halfWorldW( winW * .1 * .5 ), halfWorldH( winH * .1 * .5 );


/** \defgroup Global controls.
*/
/**@{*/

/** Number of children per scene graph node.
In a scene graph with depth 2, there would be one root node Group.
The root node would have numChildren Geode children.
*/
static int numChildren( 10 );

/** Maximum scene graph depth. Minimum valid value is 1, in which
case the scene graph consists of a single Geode. For maxDepth 2, the
scene graph consists of a single Group parent, with numChildren Geode
children. For maxDepth 3, the single Group root has numChildren Group
children, each with numChildren Geode children.
*/
static int maxDepth( 5 );

/**@}*/


/** The scene graph shares the same Geode and single Drawable.
This is a benchmark for state, not for rendering, so the Geode and
Drawable are shared to reduce the draw time cose associated with
binding the vertex buffer(s).
*/
static osg::ref_ptr< osg::Geode > sharedGeode( NULL );

osg::Node* getSharedGeode()
{
    if( sharedGeode == NULL )
    {
        osg::Geometry* geom = new osg::Geometry;
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );

        osg::Vec3Array* v = new osg::Vec3Array;
        v->push_back( osg::Vec3( -.5, -.5, 0. ) );
        v->push_back( osg::Vec3( .5, -.5, 0. ) );
        v->push_back( osg::Vec3( .5, .5, 0. ) );
        v->push_back( osg::Vec3( -.5, .5, 0. ) );
        geom->setVertexArray( v );

        osg::Vec4Array* c = new osg::Vec4Array;
        c->push_back( osg::Vec4( .9, .9, .9, 1. ) );
        c->push_back( osg::Vec4( .9, .9, .9, 1. ) );
        c->push_back( osg::Vec4( 0., 0., .9, 1. ) );
        c->push_back( osg::Vec4( 0., 0., .9, 1. ) );
        geom->setColorArray( c );
        geom->setColorBinding( osg::Geometry::BIND_PER_VERTEX );

        osg::DrawElementsUInt* de = new osg::DrawElementsUInt( GL_TRIANGLE_STRIP );
        de->push_back( 0 );
        de->push_back( 1 );
        de->push_back( 3 );
        de->push_back( 2 );
        geom->addPrimitiveSet( de );

        sharedGeode = new osg::Geode;
        sharedGeode->addDrawable( geom );
    }
    return( sharedGeode.get() );
}

/** Create the program used for benchmarking uniforms. The program
is referenced once in the scene graph, by the root node (it's not shared).
*/
osg::Program* getProgram()
{
    osg::ref_ptr< osg::Program > myProgram = new osg::Program;

    char* vShaderSource =
        "uniform float u0, u1, u2; \n" \
        "void main() { \n" \
        "  vec4 offset = vec4( u0, u1, u2, 0.0 ); \n" \
        "  gl_Position = ftransform(); \n" \
        "  gl_FrontColor = gl_Color + offset; \n" \
        "} \n";
    osg::ref_ptr< osg::Shader > vShader = new osg::Shader( osg::Shader::VERTEX,
        std::string( vShaderSource ) );
    myProgram->addShader( vShader.get() );

    char* fShaderSource =
        "void main() { \n" \
        "  gl_FragData[ 0 ] = gl_Color; \n" \
        "} \n";
    osg::ref_ptr< osg::Shader > fShader = new osg::Shader( osg::Shader::FRAGMENT,
        std::string( fShaderSource ) );
    myProgram->addShader( fShader.get() );

    return( myProgram.release() );
}

osg::Node* createSceneGraph( int depth=0 )
{
    osg::ref_ptr< osg::Node > node;
    if( depth+1 >= maxDepth )
        node = getSharedGeode();
    else
    {
        osg::ref_ptr< osg::Group > grp = new osg::Group;
        int idx;
        for( idx=0; idx<numChildren; idx++ )
            grp->addChild( createSceneGraph( depth+1 ) );
        node = grp.get();
    }
    return( node.release() );
}

class AddFFPVisitor : public osg::NodeVisitor
{
public:
    AddFFPVisitor()
        : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
    {
    }

    void apply( osg::Node& node )
    {
        osg::StateSet* ss = node.getOrCreateStateSet();
        ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        osg::CullFace* cf = new osg::CullFace( osg::CullFace::BACK );
        ss->setAttributeAndModes( cf );

        traverse( node );
    }
};
class AddUniformsVisitor : public osg::NodeVisitor
{
public:
    AddUniformsVisitor()
        : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
    {
    }

    void apply( osg::Node& node )
    {
        osg::StateSet* ss = node.getOrCreateStateSet();
        ss->addUniform( new osg::Uniform( "u0", 0.1f ) );
        ss->addUniform( new osg::Uniform( "u1", 0.09f ) );
        ss->addUniform( new osg::Uniform( "u2", 0.08f ) );

        traverse( node );
    }
};

osg::Node* testCase0()
{
    osg::notify( osg::ALWAYS ) << "Test case 0: No state." << std::endl;

    osg::ref_ptr< osg::Node > root = createSceneGraph();

    return( root.release() );
}
osg::Node* testCase1()
{
    osg::notify( osg::ALWAYS ) << "Test case 1: FFP." << std::endl;

    osg::ref_ptr< osg::Node > root = createSceneGraph();
    AddFFPVisitor ffp;
    root->accept( ffp );

    return( root.release() );
}
osg::Node* testCase2()
{
    osg::notify( osg::ALWAYS ) << "Test case 2: Shaders, but no Uniforms." << std::endl;

    osg::ref_ptr< osg::Node > root = createSceneGraph();

    // Global state: Set the program we'll be using.
    osg::StateSet* ss = root->getOrCreateStateSet();
    ss->setAttribute( getProgram() );

    return( root.release() );
}
osg::Node* testCase3()
{
    osg::notify( osg::ALWAYS ) << "Test case 3: Shaders and Uniforms." << std::endl;

    osg::ref_ptr< osg::Node > root = createSceneGraph();
    AddUniformsVisitor uniforms;
    root->accept( uniforms );

    // Global state: Set the program we'll be using.
    osg::StateSet* ss = root->getOrCreateStateSet();
    ss->setAttribute( getProgram() );

    return( root.release() );
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    int testCase( 0 );
    int testCaseParam;
    if( arguments.read( "-t", testCaseParam ) )
        testCase = testCaseParam;

    osg::ref_ptr< osg::Node > root;
    switch( testCase )
    {
    default:
        osg::notify( osg::WARN ) << "Unsupported test case: " << testCase << ". Using case 0 instead." << std::endl;
        // intentional fall-through
    case 0:
        root = testCase0();
        break;
    case 1:
        root = testCase1();
        break;
    case 2:
        root = testCase2();
        break;
    case 3:
        root = testCase3();
        break;
    }
    std::ostringstream filename;
    filename << "testCase" << testCase << ".osg";
    osgDB::writeNodeFile( *root, filename.str() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 30, 30, winW, winH );
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setSceneData( root.get() );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::ThreadingHandler );
    
    viewer.realize();
    viewer.getCamera()->setClearColor( osg::Vec4( 0., 0., 0., 1. ) );
    viewer.getCamera()->setViewMatrix( osg::Matrix::identity() );
    viewer.getCamera()->setProjectionMatrix( osg::Matrix::ortho(
        -halfWorldW, halfWorldW, -halfWorldH, halfWorldH, -1., 1. ) );

    while( !viewer.done() )
        viewer.frame();

    return( 0 );
}

