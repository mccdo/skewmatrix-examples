// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osg/MatrixTransform>
#include <osgwTools/Shapes.h>

#include <osgwTools/TransparencyUtils.h>


osg::ref_ptr< osg::Group > root;
osg::ref_ptr< osg::Node > cowModel;
osg::ref_ptr< osg::Node > teapotModel;
osg::ref_ptr< osg::Node > eightCornersModel;
osg::ref_ptr< osg::Node > surfacesModel;
osg::ref_ptr< osg::Node > boxModel;
osg::ref_ptr< osg::Node > mixedModel;

class KeyHandler: public osgGA::GUIEventHandler
{
public:
    KeyHandler() {}

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& )
    {
        if( ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN )
            return( false );

        switch( ea.getKey() )
        {
        case 'D':
            osgDB::writeNodeFile( *root, "out.osg" );
            return( true );

        case 'R':
            osgwTools::transparentDisable( root.get(), true );
            return( true );

        case 'r':
            toggle( root.get() );
            return( true );
        case 'c':
            toggle( cowModel.get() );
            return( true );
        case 't':
            toggle( teapotModel.get() );
            return( true );
        case 'e':
            toggle( eightCornersModel.get() );
            return( true );
        case 's':
            toggle( surfacesModel.get() );
            return( true );
        case 'b':
            toggle( boxModel.get() );
            return( true );
        case 'm':
            toggle( mixedModel.get() );
            return( true );
        }
        return( false );
    }
protected:
    void toggle( osg::Node* node )
    {
        if( osgwTools::isTransparent( node->getStateSet() ) )
            osgwTools::transparentDisable( node );
        else
            osgwTools::transparentEnable( node, .3f );
    }
};

osg::Node* createDefaultScene()
{
    root = new osg::Group;

    // cow
    std::string modelName = std::string( "cow.osg" );
    cowModel = osgDB::readNodeFile( modelName );
    if( !cowModel.valid() )
        osg::notify( osg::WARN ) << "Can't load model \"" << modelName << "\"" << std::endl;
    root->addChild( cowModel.get() );

    // teapot
    {
        modelName = std::string( "teapot.osg" );
        teapotModel = osgDB::readNodeFile( modelName );
        if( !teapotModel.valid() )
            osg::notify( osg::WARN ) << "Can't load model \"" << modelName << "\"" << std::endl;
        teapotModel->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );
        osg::MatrixTransform* mt = new osg::MatrixTransform(
            osg::Matrix::scale( 4., 4., 4. ) *
            osg::Matrix::translate( 10., 0., 0. )
            );
        mt->addChild( teapotModel.get() );
        root->addChild( mt );
    }

    // eight corners
    modelName = std::string( "trans-eight.stl" );
    eightCornersModel = osgDB::readNodeFile( modelName );
    if( !eightCornersModel.valid() )
        osg::notify( osg::WARN ) << "Can't load model \"" << modelName << "\"" << std::endl;
    root->addChild( eightCornersModel.get() );

    // surfaces
    {
        modelName = std::string( "trans-surface.stl" );
        surfacesModel = osgDB::readNodeFile( modelName );
        if( !surfacesModel.valid() )
            osg::notify( osg::WARN ) << "Can't load model \"" << modelName << "\"" << std::endl;
        osg::MatrixTransform* mt = new osg::MatrixTransform( osg::Matrix::translate( 0., 0., -6. ) );
        mt->addChild( surfacesModel.get() );
        root->addChild( mt );
    }

    // box
    {
        // The box Drawable has blending off. This means if we toggle transparency on
        // the geode, nothing will happen -- except we will use the OVERRIDE bit
        // to force transparency.
        osg::Geode* geode = new osg::Geode;
        boxModel = geode;
        osg::Drawable* drawable = osgwTools::makeBox( osg::Vec3( 2., 1., 1. ) );
        drawable->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::OFF );
        geode->addDrawable( drawable );
        osg::MatrixTransform* mt = new osg::MatrixTransform( osg::Matrix::translate( 8., 0., 4. ) );
        mt->addChild( boxModel.get() );
        root->addChild( mt );
    }

    // mixed -- test a model that contains transparent geometry
    modelName = std::string( "trans-mixed.osg" );
    mixedModel = osgDB::readNodeFile( modelName );
    if( !mixedModel.valid() )
        osg::notify( osg::WARN ) << "Can't load model \"" << modelName << "\"" << std::endl;
    root->addChild( mixedModel.get() );
    osgwTools::ProtectTransparencyVisitor ptv;
    mixedModel->accept( ptv );

    return( root.get() );
}


int
main( int argc, char** argv )
{
    osg::ref_ptr< osg::Node > scene = createDefaultScene();

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
    viewer.addEventHandler( new KeyHandler );
    viewer.setSceneData( scene.get() );
    viewer.run();
}
