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
#include <osg/Quat>
#include <osg/StateSet>
#include <osg/PositionAttitudeTransform>

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

bool configureCircleSettings(const osg::Vec3 eyePoint, const osg::NodePath& nodePath, double &outSubdivision, double &outRadius)
{
	// <<<>>> Paul: Replace with code to actually calculate these
	outSubdivision = 64;
	outRadius = 10.0;
	osg::BoundingSphere sphere;
	osg::Node *node = nodePath[nodePath.size()-1];
	if(node)
	{
		sphere = node->getBound();
		outRadius = sphere.radius();
	} // if
	return true;
} // configureCircleSettings

osg::Node *createCircleHighlight(const osg::Vec3 eyePoint, const osg::NodePath& nodePath, const std::string textAnnotation)
{
	double Subdivision, Radius;

	// determine Subdivision and Radius settings for current viewpoint (stub for now)
	configureCircleSettings(eyePoint, nodePath, Subdivision, Radius);

	// Structure: AbsoluteModelTransform->AutoTransform->CirclePAT->CircleGeode->Circle
	//                                \-->TextPAT->AutoTransform->TextGeode->Text
    osg::ref_ptr< osg::Geode > circlegeode;
    osg::ref_ptr< osg::AutoTransform > circleat;
	osg::ref_ptr< osg::PositionAttitudeTransform > circlepat;
    osg::ref_ptr< osgwTools::AbsoluteModelTransform > amt;

	// determine position of highlight
	osg::Vec3 position(0., 0., 0.);
	osg::Matrix matrix = osg::computeLocalToWorld(nodePath);
	position = nodePath[nodePath.size()-1]->getBound().center();

	circlegeode = new osg::Geode;
    circlegeode->addDrawable( osgwTools::makeWireCircle( Radius, Subdivision) );
    circleat = new osg::AutoTransform();
	circlepat = new osg::PositionAttitudeTransform;
    circlepat->addChild( circlegeode.get() );
    circleat->addChild( circlepat.get() );
    circleat->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
    circleat->setAutoScaleToScreen(false);
    circleat->setPosition(position);
	amt = new osgwTools::AbsoluteModelTransform;
	amt->addChild( circleat.get() );
	amt->setMatrix(matrix); // setup Absolute Model Transform to mimic transforms of nodepath
	// deal with default orientation of osgwTools:Shape circle not being suitable for AutoTransform
	circlepat->setAttitude(osg::Quat(90.0f, osg::Vec3d(1., 0., 0.)));

	// turn off depth testing on our subgraph
	amt->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );
	amt->getOrCreateStateSet()->setRenderBinDetails(25, "RenderBin");
	amt->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );

	if(!textAnnotation.empty())
	{
	    osg::ref_ptr< osg::AutoTransform > textat;
		osg::ref_ptr< osg::PositionAttitudeTransform > textpat;
	    osg::ref_ptr< osg::Geode > textgeode;
		// Add text annotation
	    textat = new osg::AutoTransform();
		osg::ref_ptr<osgText::Text> text = new osgText::Text;
		std::string timesFont("fonts/arial.ttf");
		text->setCharacterSize(15); // need to be variable?
		text->setText(textAnnotation);
		text->setFont(timesFont);
		text->setColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
		text->setAlignment(osgText::Text::CENTER_BASE_LINE);
		textgeode = new osg::Geode;
		textgeode->addDrawable( text.get() );
		textpat = new osg::PositionAttitudeTransform;
		textpat->addChild( textat.get() );
		textpat->setPosition(osg::Vec3(0, Radius, 0));
		amt->addChild( textpat.get() );
		textat->addChild( textgeode.get() );
		textat->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
		textat->setAutoScaleToScreen(true);
		textat->setPosition(position);
	} // if

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
                    pick(ea,viewer);
                } // if
                return true;
            } // RELEASE

            default:
                return false;
        } // switch event type
    } // handle

    void pick(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer)
    {
        osg::Node* scene = viewer->getSceneData();
        if (!scene) return;

        osg::notify(osg::NOTICE)<<std::endl;

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

            osg::notify(osg::NOTICE)<<"Picked "<<intersection.localIntersectionPoint<<std::endl
                <<"  Distance to ref. plane "<<intersection.distance
                <<", max. dist "<<intersection.maxDistance
                <<", primitive index "<<intersection.primitiveIndex
                <<", numIntersectionPoints "
                <<intersection.numIntersectionPoints
                <<std::endl;

            osg::NodePath& nodePath = intersection.nodePath;
            node = (nodePath.size()>=1)?nodePath[nodePath.size()-1]:0;
            parent = (nodePath.size()>=2)?dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]):0;

			if (node)
			{
				std::cout<<"  Hits "<<node->className()<< " named " << node->getName() << ". nodePath size "<<nodePath.size()<<std::endl;    

				// highlighting
				static osg::ref_ptr<osg::Node> currentHighlight;

				// remove existing highlight if necessary
				if(currentHighlight.valid())
				{
					viewer->getSceneData()->asGroup()->removeChild(currentHighlight.get());
					currentHighlight = 0; // dispose of it
				} // if

				osg::ref_ptr<osg::Node> highlightGraph = createCircleHighlight(osg::Vec3(0., 0., 0.), nodePath, node->getName());
				if(viewer->getSceneData()->asGroup())
				{
					viewer->getSceneData()->asGroup()->addChild(highlightGraph);
					currentHighlight = highlightGraph;
				} // if
			} // if

        } // if intersections
    } // pick

protected:

    float _mx,_my;
}; // PickHandler

int main( int argc, char **argv )
{
    osg::ref_ptr<osg::Node> loadedModel;
    
    // load the scene.
    if (argc>1) loadedModel = osgDB::readNodeFile(argv[1]);
    
    // if not loaded assume no arguments passed in, try use default mode instead.
//    if (!loadedModel) loadedModel = osgDB::readNodeFile("dumptruck.osg");
    if (!loadedModel) loadedModel = osgDB::readNodeFile("C:\\Users\\Xenon\\Documents\\AlphaPixel\\Documents\\Contracting\\Skew\\Training\\BallAero\\321893main_ISS-hi-res-lwo\\ISS models 2008\\15A-noanim.ive");
    
    if (!loadedModel) 
    {
        std::cout << argv[0] <<": No data loaded." << std::endl;
        return 1;
    } // if
    
    // create the window to draw to.
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 200;
    traits->y = 200;
    traits->width = 800;
    traits->height = 600;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    osgViewer::GraphicsWindow* gw = dynamic_cast<osgViewer::GraphicsWindow*>(gc.get());
    if (!gw)
    {
        osg::notify(osg::NOTICE)<<"Error: unable to create graphics window."<<std::endl;
        return 1;
    } // if

    // create the view of the scene.
    osgViewer::Viewer viewer;
    viewer.getCamera()->setGraphicsContext(gc.get());
    viewer.getCamera()->setViewport(0,0,800,600);
    viewer.setSceneData(loadedModel.get());
    
    // create a tracball manipulator to move the camera around in response to keyboard/mouse events
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    osg::ref_ptr<osgGA::StateSetManipulator> statesetManipulator = new osgGA::StateSetManipulator(viewer.getCamera()->getStateSet());
    viewer.addEventHandler(statesetManipulator.get());

    // add the pick handler
    viewer.addEventHandler(new PickHandler());

    viewer.realize();

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while(!viewer.done())
    {
        viewer.frame();
    } // while

    return 0;
} // main

