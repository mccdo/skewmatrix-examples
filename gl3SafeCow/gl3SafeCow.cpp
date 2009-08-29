// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <osg/NodeVisitor>
#include <osg/Uniform>
#include <osg/Matrix>


class MakeGeneric : public osg::NodeVisitor
{
public:
    MakeGeneric( osg::Program& prog )
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _prog( prog ) {}
    ~MakeGeneric() {}

    void apply( osg::Geode& geode )
    {
        unsigned int idx;
        for( idx=0; idx<geode.getNumDrawables(); idx++ )
        {
            osg::Drawable* draw = geode.getDrawable( idx );
            draw->setUseDisplayList( false );
            draw->setUseVertexBufferObjects( true );

            osg::Geometry* geom = draw->asGeometry();
            if( geom == NULL ) continue;

            geom->setVertexAttribData( 0,
                osg::Geometry::ArrayData( geom->getVertexArray(), osg::Geometry::BIND_PER_VERTEX ) );
            geom->setVertexAttribData( 1,
                osg::Geometry::ArrayData( geom->getNormalArray(), osg::Geometry::BIND_PER_VERTEX ) );
        }
    }

protected:
    const osg::Program& _prog;
};


osg::ref_ptr< osg::Uniform > projMatUni;
osg::ref_ptr< osg::Uniform > viewMatUni;
osg::ref_ptr< osg::Uniform > normalMatUni;

void
addShaders( osg::Node* node )
{
    osg::StateSet* ss = node->getOrCreateStateSet();

    osg::ref_ptr< osg::Shader > vertShader = osg::Shader::readShaderFile(
        osg::Shader::VERTEX, osgDB::findDataFile( "gl3SafeCow.vs" ) );
    osg::ref_ptr< osg::Shader > fragShader = osg::Shader::readShaderFile(
        osg::Shader::FRAGMENT, osgDB::findDataFile( "gl3SafeCow.fs" ) );

    osg::ref_ptr< osg::Program > program = new osg::Program();
    program->addShader( vertShader.get() );
    program->addShader( fragShader.get() );
    ss->setAttribute( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    projMatUni = new osg::Uniform( "projMat", osg::Matrix::identity() );
    projMatUni->setDataVariance( osg::Object::DYNAMIC );
    ss->addUniform( projMatUni.get() );

    viewMatUni = new osg::Uniform( "viewMat", osg::Matrix::identity() );
    viewMatUni->setDataVariance( osg::Object::DYNAMIC );
    ss->addUniform( viewMatUni.get() );

    osg::Matrix3 normalMat;
    normalMatUni = new osg::Uniform( "normalMat", normalMat );
    normalMatUni->setDataVariance( osg::Object::DYNAMIC );
    ss->addUniform( normalMatUni.get() );

    ss->addUniform( new osg::Uniform( "sphereTex", 0 ) );

    MakeGeneric mg( *program );
    node->accept( mg );
    osg::notify( osg::INFO ) << std::endl << std::endl << "Just ran MakeGeneric" << std::endl << std::endl;
}


int
main( int argc, char** argv )
{
    osg::ref_ptr< osg::Node > root = osgDB::readNodeFile( "cow.osg" );
    addShaders( root.get() );

    osgViewer::Viewer viewer;
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 100, 100, 640, 480 );
    viewer.setSceneData( root.get() );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    while (!viewer.done())
    {
        // Update uniforms.
        // ...Projection matrix.
        projMatUni->set( viewer.getCamera()->getProjectionMatrix() );

        // ...View matrix.
        osg::Matrix viewMat = viewer.getCamera()->getViewMatrix();
        viewMatUni->set( viewMat );

        // ...Normal matrix.
        osg::Matrix3 normalMat( viewMat(0,0), viewMat(0,1), viewMat(0,2),
            viewMat(1,0), viewMat(1,1), viewMat(1,2),
            viewMat(2,0), viewMat(2,1), viewMat(2,2) );
        normalMatUni->set( normalMat );


        viewer.frame();
    }

    return( 0 );
}

