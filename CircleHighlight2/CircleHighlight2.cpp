// Copyright (c) 2010 Skew Matrix Software LLC. All rights reserved.

/* Example, CircleHighlight
* 
* Based originally on OSG example osgkeyboardmouse's PolytopeIntersector code
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

// Simple example of use of picking and highlighting with a circle

#include "CircleSupport.h"

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PolytopeIntersector>

#include <osg/NodeVisitor>
#include <osg/StateSet>

#include <iostream>
#include <osg/io_utils>


#define NOT_PICKABLE_NODE_MASK  0x80000000

class ApplyNodeMask : public osg::NodeVisitor
{
public:
    ApplyNodeMask( unsigned int nodeMask )
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _nodeMask( nodeMask )
    {}

    virtual void apply( osg::Node& node )
    {
        node.setNodeMask( _nodeMask );
        traverse( node );
    }

protected:
    unsigned int _nodeMask;
};


// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler, public CircleSupport
{
public: 
    PickHandler( osg::Group* labelGroup )
      : _labelGroup( labelGroup ),
        _pickMode( PICK_GROUP ),
        _labelMode( LABEL_USER_SPECIFIED ),
        _mx(0.0),
        _my(0.0)
    {
        createCircleState( _labelGroup->getOrCreateStateSet() );
        _labelGroup->setNodeMask( NOT_PICKABLE_NODE_MASK );
    }
    ~PickHandler() {}

    // Default: PICK_GROUP
    typedef enum {
        PICK_GROUP,
        PICK_GEODE,
        PICK_DRAWABLE
    } PickMode;
    void setPickMode( const PickMode pickMode ) { _pickMode = pickMode; }
    PickMode getPickMode() const { return( _pickMode ); }

    // Default: LABEL_USER_SPECIFIED
    typedef enum {
        LABEL_USER_SPECIFIED,
        LABEL_FROM_OBJECT
    } LabelMode;
    void setLabelMode( const LabelMode labelMode ) { _labelMode = labelMode; }
    LabelMode getLabelMode() const { return( _labelMode ); }

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        if (!viewer) return false;

        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::PUSH):
            case(osgGA::GUIEventAdapter::MOVE):
            {
                _mx = ea.getX();
                _my = ea.getY();

                // false: pass to camera manipulator.
                return false;
            } // PUSH/MOVE
            case(osgGA::GUIEventAdapter::RELEASE):
            {
                if (_mx == ea.getX() && _my == ea.getY())
                {
                    // only do a pick if the mouse hasn't moved
                    osg::ref_ptr< osg::Node > subgraph( pick( ea, viewer ) );
                    if( (subgraph != NULL) && (_labelGroup != NULL) )
                    {
                        _labelGroup->addChild( subgraph.get() );

                        // HACK to work around a bug in OSG. Only attach a program
                        // once the subgraph has something to draw.
                        osg::StateSet* ss = _labelGroup->getOrCreateStateSet();
                        if( ss->getAttribute( osg::StateAttribute::PROGRAM ) == NULL )
                            ss->setAttribute( _lineStripProgram );
                    }
                } // if

                // false: pass to camera manipulator.
                return false;
            } // RELEASE

            case( osgGA::GUIEventAdapter::KEYDOWN ):
            {
                switch( ea.getKey() )
                {
                    case( osgGA::GUIEventAdapter::KEY_Delete ):
                    {
                        _labelGroup->removeChildren( 0, _labelGroup->getNumChildren() );

                        // HACK to work around a bug in OSG. If a program is attached to a node
                        // that has no Drawables in its subgraph, that program will "leak" out to
                        // other sibling nodes and affect their rendering. So, when we remove
                        // all the circle highlights, we must also remove the program.
                        _labelGroup->getOrCreateStateSet()->removeAttribute( osg::StateAttribute::PROGRAM );

                        return( true );
                    }
                    case( 't' ):
                    case( 'T' ):
                    {
                        if( _labelGroup->getNodeMask() == 0 )
                            _labelGroup->setNodeMask( ~NOT_PICKABLE_NODE_MASK );
                        else
                            _labelGroup->setNodeMask( 0 );
                        return( true );
                    }
                }
                return( false );
            }

            default:
                return false;
        } // switch event type
    } // handle



protected:
    osg::Node* pick( const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer );

    float _mx, _my;

    osg::ref_ptr< osg::Group > _labelGroup;

    PickMode _pickMode;
    LabelMode _labelMode;
}; // PickHandler


osg::Node*
PickHandler::pick( const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer )
{
    osg::notify( osg::DEBUG_FP )<<std::endl;

    osg::Node* node = 0;
    osg::Group* parent = 0;

    osgUtil::PolytopeIntersector* picker;
    double mx = ea.getXnormalized();
    double my = ea.getYnormalized();
    double w = 0.05;
    double h = 0.05;
    picker = new osgUtil::PolytopeIntersector( osgUtil::Intersector::PROJECTION, mx-w, my-h, mx+w, my+h );

    osgUtil::IntersectionVisitor iv(picker);
    iv.setTraversalMask( ~NOT_PICKABLE_NODE_MASK );

    viewer->getCamera()->accept(iv);

    if (picker->containsIntersections())
    {
        osgUtil::PolytopeIntersector::Intersection intersection = picker->getFirstIntersection();

        osg::notify( osg::DEBUG_FP )<<"Picked "<<intersection.localIntersectionPoint<<std::endl
            <<"  Distance to ref. plane "<<intersection.distance
            <<", max. dist "<<intersection.maxDistance
            <<", primitive index "<<intersection.primitiveIndex
            <<", numIntersectionPoints "
            <<intersection.numIntersectionPoints
            <<std::endl;

        std::string objectName;
        const osg::NodePath& nodePath = intersection.nodePath;
        osg::BoundingSphere sphere;
        switch( _pickMode )
        {
        case PICK_DRAWABLE:
            if( intersection.drawable != NULL )
            {
                sphere = intersection.drawable->getBound();
                objectName = intersection.drawable->getName();
                break;
            }
            // intentional fallthrough
        case PICK_GROUP:
            if( nodePath.size() > 1 )
            {
                sphere = nodePath[ nodePath.size()-2 ]->getBound();
                break;
            }
            // intentional fallthrough
        case PICK_GEODE:
            sphere = nodePath[ nodePath.size()-1 ]->getBound();
            objectName = nodePath[ nodePath.size()-1 ]->getName();
            break;
        }

        if( _labelMode == LABEL_FROM_OBJECT )
        {
            if( objectName.empty() )
            {
                osg::NodePath::const_reverse_iterator it = nodePath.rbegin();
                it++;
                for( ; it != nodePath.rend(); it++ )
                {
                    objectName = (*it)->getName();
                    if( !( objectName.empty() ) )
                        break;
                }
            }
            setLabelText( objectName );
        }

        osg::Vec3 eyepoint, center, up;
        viewer->getCamera()->getViewMatrixAsLookAt( eyepoint, center, up );
        osg::ref_ptr<osg::Node> highlightGraph = createCircleHighlight( nodePath, sphere );

        return( highlightGraph.release() );
    } // if intersections

    return( NULL );
} // pick



int main( int argc, char **argv )
{
    osg::ref_ptr< osg::Group > root( new osg::Group );

    osg::ref_ptr< osg::Node > loadedModel( NULL );
    // load the scene.
    if (argc>1)
        loadedModel = osgDB::readNodeFile(argv[1]);
    if (!loadedModel)
        loadedModel = osgDB::readNodeFile("C:\\Users\\Xenon\\Documents\\AlphaPixel\\Documents\\Contracting\\Skew\\Training\\BallAero\\321893main_ISS-hi-res-lwo\\ISS models 2008\\15A-noanim.ive");
    if (!loadedModel)
        loadedModel = osgDB::readNodeFile( "cow.osg" );
    if (!loadedModel) 
    {
        osg::notify( osg::FATAL ) << argv[0] <<": No data loaded." << std::endl;
        return 1;
    } // if
    root->addChild( loadedModel.get() );

    {
        ApplyNodeMask anm( ~NOT_PICKABLE_NODE_MASK );
        root->accept( anm );
    }

    osg::ref_ptr< osg::Group > labelGroup( new osg::Group );
    root->addChild( labelGroup.get() );

    // create the view of the scene.
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
    viewer.setSceneData( root.get() );

    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator(viewer.getCamera()->getStateSet());
    viewer.addEventHandler(statesetManipulator.get());

    // add the pick handler
    PickHandler* ph = new PickHandler( labelGroup.get() );
    ph->setPickMode( PickHandler::PICK_GROUP );
    ph->setLabelMode( PickHandler::LABEL_FROM_OBJECT );
    viewer.addEventHandler( ph );

    viewer.realize();

    // Turn on error checking per StateAttribute
    osgViewer::Viewer::Windows windows;
    viewer.getWindows( windows );
    osgViewer::Viewer::Windows::iterator itr;
    for( itr = windows.begin(); itr != windows.end(); itr++ )
        (*itr)->getState()->setCheckForGLErrors( osg::State::ONCE_PER_ATTRIBUTE );

    // Sets the viewport width and height as a uniform.
    viewer.getCamera()->setUpdateCallback( new CameraUpdateViewportCallback() );


    osg::notify( osg::ALWAYS ) << "Click on the model to highlight." << std::endl;
    osg::notify( osg::ALWAYS ) << "\tT/t\ttoggle highlights on/off." << std::endl;
    osg::notify( osg::ALWAYS ) << "\tDel\tClear all highlights." << std::endl;

    return( viewer.run() );
} // main

