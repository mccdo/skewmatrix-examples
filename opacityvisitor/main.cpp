// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osg/MatrixTransform>

#include "TransparencySupport.h"


osg::ref_ptr< osg::Group > root;
osg::ref_ptr< osg::Node > cowModel;
osg::ref_ptr< osg::Node > teapotModel;


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
        case 'c':
            toggle( cowModel.get() );
            return( true );
        case 't':
            toggle( teapotModel.get() );
            return( true );
        case 'r':
            toggle( root.get() );
            return( true );
        }
        return( false );
    }
protected:
    void toggle( osg::Node* node )
    {
        if( isTransparent( node ) )
            transparentDisable( node );
        else
            transparentEnable( node, .25f );
    }
};

osg::Node* createDefaultScene()
{
    root = new osg::Group;

    cowModel = osgDB::readNodeFile( "cow.osg" );
    root->addChild( cowModel.get() );

    teapotModel = osgDB::readNodeFile( "teapot.osg" );
    teapotModel->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );
    osg::MatrixTransform* mt = new osg::MatrixTransform(
        osg::Matrix::scale( 4., 4., 4. ) *
        osg::Matrix::translate( 10., 0., 0. )
        );
    mt->addChild( teapotModel.get() );
    root->addChild( mt );

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
