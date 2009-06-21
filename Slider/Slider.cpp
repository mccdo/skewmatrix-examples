// Copyright (c) 2009 e LLC. All rights reserved.
//

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osg/MatrixTransform>
#include <osg/NodeCallback>

#include <osgControls/SliderControl.h>


class SliderUpdate : public osg::NodeCallback
{
public:
    SliderUpdate( osgControls::SliderControl* sc )
      : _sc( sc )
    {}

    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        osg::MatrixTransform* mt( dynamic_cast< osg::MatrixTransform* >( node ) );
        const float v( _sc->getCurrentValue() );
        mt->setMatrix( osg::Matrix::scale( v, v, v ) );

        traverse( node, nv );
    }

protected:
    osgControls::SliderControl* _sc;
};

int
main( int argc,
      char ** argv )
{
    osg::Group* root = new osg::Group;

    osgControls::SliderControl* sc = new osgControls::SliderControl;
    sc->setDisplayArea( 7. );
    sc->setValueRange( .5, 3. );
    sc->setTimeRange( 3. );
    sc->setCurrentValue( 1. );

    osg::MatrixTransform* mt = new osg::MatrixTransform( osg::Matrix::translate( 0., -8., 0. ) );
    mt->addChild( sc->getSliderControlSubgraph() );
    root->addChild( mt );

    mt = new osg::MatrixTransform;
    mt->setUpdateCallback( new SliderUpdate( sc ) );
    root->addChild( mt );
    mt->addChild( osgDB::readNodeFile( "cow.osg" ) );


    osgViewer::Viewer viewer;
    //viewer.setUpViewOnSingleScreen( 0 );
    viewer.setUpViewInWindow( 1000, 400, 640, 480 );
    viewer.setSceneData( root );

    return( viewer.run() );
}

