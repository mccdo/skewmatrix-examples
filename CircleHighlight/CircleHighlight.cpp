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

#include <osg/io_utils>

#include <osg/AutoTransform>
#include <osg/StateSet>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PolytopeIntersector>

#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <osgViewer/Viewer>

#include <osgText/Text>
#include <osgText/Font>

#include <osgwTools/AbsoluteModelTransform.h>
#include <osgwTools/Shapes.h>

#include <iostream>

int getCircleSubdivisions( double distanceOverRadius )
{
    // Subdivision segments is inversely proportional to distance/radius.
    // If distance/radiue if halved, segments is doubled, and vice versa.
    // Basis: subdivide circle with 60 segments at a distance/radius of 10 units.
    int outSubdivision = (int)( 10.f / distanceOverRadius * 60.f );

    osg::notify( osg::DEBUG_FP ) << "  Using subdiv " << outSubdivision << std::endl;

    return( outSubdivision );
} // getCircleSubdivisions

osg::Node *createCircleHighlight(const osg::Vec3 eyePoint, const osg::NodePath& nodePath,
                                 const osg::Node& pickedNode, const std::string& labelText )
{
    const std::string textAnnotation( labelText );

    // determine Subdivision and Radius settings for current viewpoint (stub for now)
    osg::BoundingSphere sphere( pickedNode.getBound() );
    const double radius( sphere.radius() );
    osg::Vec3 dVec( sphere.center() - eyePoint );
    const double distance( dVec.length() );
    const int subdivisions( getCircleSubdivisions( distance / radius ) );

    // Determine text pos and line segment endpoints.
    osg::Vec3 textDirection( 1., 1., 0. );
    textDirection.normalize();
    osg::Vec3 lineEnd( textDirection * radius );
    osg::Vec3 textPos( textDirection * radius * 1.4f );

    // Structure:
    //   AbsoluteModelTransform->AutoTransform->CircleGeode->Circle (Geometry)
    //                                                   \-->Line segment (Geometry)
    //                                                   \-->Label (osgText::Text)
    osg::ref_ptr< osg::Geode > circlegeode;
    osg::ref_ptr< osg::AutoTransform > circleat;
    osg::ref_ptr< osgwTools::AbsoluteModelTransform > amt;

    // determine position of highlight
    osg::Vec3 position(0., 0., 0.);
    osg::Matrix matrix = osg::computeLocalToWorld(nodePath);
    position = nodePath[nodePath.size()-1]->getBound().center();

    circlegeode = new osg::Geode;
    osg::Geometry* circleGeom( osgwTools::makeWireCircle( radius, subdivisions ) );
    circlegeode->addDrawable( circleGeom );
    circleat = new osg::AutoTransform();
    circleat->addChild( circlegeode.get() );
    circleat->setAutoRotateMode( osg::AutoTransform::ROTATE_TO_CAMERA );
    circleat->setAutoScaleToScreen(false);
    circleat->setPosition(position);
    amt = new osgwTools::AbsoluteModelTransform;
    amt->addChild( circleat.get() );
    amt->setMatrix(matrix); // setup Absolute Model Transform to mimic transforms of nodepath

    // Add a line segment from the circle to the text.
    {
        osg::Geometry* lineGeom = new osg::Geometry;
        circlegeode->addDrawable( lineGeom );

        osg::Vec3Array* verts( new osg::Vec3Array );
        verts->resize( 2 );
        (*verts)[ 0 ] = lineEnd;
        (*verts)[ 1 ] = textPos;
        lineGeom->setVertexArray( verts );

        lineGeom->setColorArray( circleGeom->getColorArray() );
        lineGeom->setColorBinding( osg::Geometry::BIND_OVERALL );

        lineGeom->addPrimitiveSet( new osg::DrawArrays( GL_LINES, 0, 2 ) );
    }

    if(!textAnnotation.empty())
    {
        // Add text annotation
        osg::ref_ptr<osgText::Text> text = new osgText::Text;
        text->setPosition( textPos );
        text->setFont( "arial.ttf" );
        text->setText( textAnnotation );
        text->setColor( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        text->setAlignment( osgText::Text::LEFT_BOTTOM );
        text->setAxisAlignment( osgText::Text::XY_PLANE );

        // Character size goes up as a function of distance.
        // Basis: Size is 0.1 for a distance of 10.0.
        float size( 0.01f * distance );
        osg::notify( osg::DEBUG_FP ) << "    Using char size " << size << std::endl;
        text->setCharacterSize( size );

        circlegeode->addDrawable( text.get() );
    } // if

    // TBD application responsibility?
    // turn off depth testing on our subgraph
    amt->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );
    amt->getOrCreateStateSet()->setRenderBinDetails( 1000, "RenderBin" );
    amt->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );

    return ( amt.release() );
} // createCircleHighlight

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler 
{
public: 

    PickHandler():
        _mx(0.0),_my(0.0) {}

    ~PickHandler() {}

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
                return false;
            } // PUSH/MOVE
            case(osgGA::GUIEventAdapter::RELEASE):
            {
                if (_mx == ea.getX() && _my == ea.getY())
                {
                    // only do a pick if the mouse hasn't moved
                    pick( ea, viewer );
                } // if

                // return false so that TrackballManipulator 'throw' works.
                return false;
            } // RELEASE

            default:
                return false;
        } // switch event type
    } // handle

    void pick( const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer )
    {
        osg::Node* scene = viewer->getSceneData();
        if (!scene) return;

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

            osg::NodePath& nodePath = intersection.nodePath;
            node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
            parent = (nodePath.size()>=2)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]):0;

            osg::Node* pickedNode( node );
            if( (node->asGroup() == NULL) && (parent != NULL) )
                pickedNode = parent;
            if( pickedNode )
            {
                 osg::notify( osg::DEBUG_FP ) <<"  Hits "<< pickedNode->className() << " named " << pickedNode->getName() << ". nodePath size "<<nodePath.size()<<std::endl;

                // highlighting
                static osg::ref_ptr<osg::Node> currentHighlight;

                // remove existing highlight if necessary
                if(currentHighlight.valid())
                {
                    viewer->getSceneData()->asGroup()->removeChild(currentHighlight.get());
                    currentHighlight = 0; // dispose of it
                } // if

                osg::Vec3 eyepoint, center, up;
                viewer->getCamera()->getViewMatrixAsLookAt( eyepoint, center, up );
                osg::ref_ptr<osg::Node> highlightGraph = createCircleHighlight(
                    eyepoint, nodePath, *pickedNode, _labelText );
                if(viewer->getSceneData()->asGroup())
                {
                    viewer->getSceneData()->asGroup()->addChild(highlightGraph);
                    currentHighlight = highlightGraph;
                } // if
            } // if

        } // if intersections
    } // pick

    void setLabelText( const std::string& labelText )
    {
        _labelText = labelText;
    }

    const std::string& getLabelText() const
    {
        return( _labelText );
    }

protected:
    float _mx,_my;

    std::string _labelText;
}; // PickHandler

int main( int argc, char **argv )
{
    osg::ref_ptr<osg::Node> loadedModel( NULL );
    
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
    
    // create the view of the scene.
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 30, 30, 800, 600 );
    viewer.setSceneData(loadedModel.get());

    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator(viewer.getCamera()->getStateSet());
    viewer.addEventHandler(statesetManipulator.get());

    // add the pick handler
    PickHandler* ph = new PickHandler();
    viewer.addEventHandler( ph );
    ph->setLabelText( "test label 01234" );

    viewer.realize();

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while(!viewer.done())
    {
        viewer.frame();
    } // while

    return 0;
} // main

