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

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/PolytopeIntersector>

#include <osg/NodeVisitor>
#include <osg/StateSet>
#include <osg/Shader>
#include <osgText/Text>
#include <osgText/Font>
#include <osgwTools/AbsoluteModelTransform.h>
#include <osgwTools/Shapes.h>

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
class PickHandler : public osgGA::GUIEventHandler 
{
public: 
    PickHandler( osg::Group* labelGroup )
      : _labelGroup( labelGroup ),
        _mx(0.0),
        _my(0.0)
    {
        createCircleState( _labelGroup->getOrCreateStateSet() );
    }
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
                            ss->setAttribute( _program );
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


    // App needs to explicitly set a text label, and the event handler
    // will use it for the next pick. Alternatively, the event handler
    // could be modified to support a callback, written by the app, to
    // supply the text label.
    void setLabelText( const std::string& labelText )
    {
        _labelText = labelText;
    }

    const std::string& getLabelText() const
    {
        return( _labelText );
    }

protected:
    osg::Node* pick( const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer );
    osg::Node* createCircleHighlight( const osg::Vec3 eyePoint, const osg::NodePath& nodePath,
        const osg::Node& pickedNode, const std::string& labelText );
    void createCircleState( osg::StateSet* ss );

    float _mx, _my;

    std::string _labelText;

    osg::ref_ptr< osg::Group > _labelGroup;
    osg::ref_ptr< osg::Program > _program;
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

        osg::NodePath& nodePath = intersection.nodePath;
        node = (nodePath.size()>=1) ? nodePath[nodePath.size()-1] : 0;
        parent = (nodePath.size()>=2) ? dynamic_cast<osg::Group*>(nodePath[nodePath.size()-2]) : 0;

        osg::Node* pickedNode( node );
#if 0
        // Enable this code to force picking a group.
        // Personally, I like being able to pick Geodes.
        if( (node->asGroup() == NULL) && (parent != NULL) )
            pickedNode = parent;
#endif
        if( pickedNode )
        {
            osg::notify( osg::DEBUG_FP ) <<"  Hits "<< pickedNode->className() << " named " << pickedNode->getName() << ". nodePath size "<<nodePath.size()<<std::endl;

            osg::Vec3 eyepoint, center, up;
            viewer->getCamera()->getViewMatrixAsLookAt( eyepoint, center, up );
            osg::ref_ptr<osg::Node> highlightGraph = createCircleHighlight(
                eyepoint, nodePath, *pickedNode, _labelText );

            return( highlightGraph.release() );
        } // if
    } // if intersections

    return( NULL );
} // pick


osg::Node*
PickHandler::createCircleHighlight( const osg::Vec3 eyePoint, const osg::NodePath& nodePath,
                                   const osg::Node& pickedNode, const std::string& labelText )
{
    const std::string textAnnotation( labelText );

    // determine Subdivision and Radius settings for current viewpoint (stub for now)
    osg::BoundingSphere sphere( pickedNode.getBound() );
    const double radius( sphere.radius() );
    osg::Vec3 dVec( sphere.center() - eyePoint );
    const double distance( dVec.length() );

    osg::notify( osg::ALWAYS ) << "Picked node name: " << pickedNode.getName() << std::endl;
    osg::notify( osg::ALWAYS ) << "  radius: " << sphere.radius() << std::endl;

    // Subdivision segments is inversely proportional to distance/radius.
    // If distance/radiue if halved, segments is doubled, and vice versa.
    // Basis: subdivide circle with 60 segments at a distance/radius of 10 units.
    const int subdivisions( (int)( 10.f / ( distance / radius ) * 60.f ) );
    osg::notify( osg::DEBUG_FP ) << "  Using subdiv " << subdivisions << std::endl;

    // Determine text pos and line segment endpoints.
    // In xy (z=0) plane.
    osg::Vec3 textDirection( 1., 1., 0. );
    textDirection.normalize();
    osg::Vec3 lineEnd( textDirection * radius );
    osg::Vec3 textPos( textDirection * radius * 1.4f );

    // Structure:
    //   AbsoluteModelTransform->CircleGeode-->Circle (Geometry)
    //                                   \---->Line segment (Geometry)
    //                                    \--->Label (osgText::Text)
    osg::ref_ptr< osg::Geode > circlegeode;
    osg::ref_ptr< osgwTools::AbsoluteModelTransform > amt;

    // determine position of highlight
    osg::Vec3 position = sphere.center();
    osg::Matrix matrix = osg::computeLocalToWorld( nodePath ) *
        osg::Matrix::translate( position );

    circlegeode = new osg::Geode;
    // By default, circle is created in xy (z=0) plane.
    osg::Geometry* circleGeom( osgwTools::makeWireCircle( radius, subdivisions ) );
    circlegeode->addDrawable( circleGeom );
    amt = new osgwTools::AbsoluteModelTransform;
    amt->setNodeMask( NOT_PICKABLE_NODE_MASK );
    amt->addChild( circlegeode.get() );
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
        // In xy (z=0) plans.
        text->setAxisAlignment( osgText::Text::XY_PLANE );

        // This is how we render osgText when we have our own shader.
        // See circle.fs for shader code.
        text->getOrCreateStateSet()->addUniform(
            new osg::Uniform( "circleEnableText", 1 ) );

        // Character size goes up as a function of distance.
        // Basis: Size is 0.1 for a distance of 10.0.
        float size( 0.01f * distance );
        osg::notify( osg::DEBUG_FP ) << "    Using char size " << size << std::endl;
        text->setCharacterSize( size );

        circlegeode->addDrawable( text.get() );
    } // if

    return( amt.release() );
} // createCircleHighlight

void
PickHandler::createCircleState( osg::StateSet* ss )
{
    // turn off depth testing on our subgraph
    ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );
    ss->setRenderBinDetails( 1000, "RenderBin" );

    // Support for texture mapped text. Off by default, but test stateset will enable it.
    ss->addUniform( new osg::Uniform( "circleEnableText", 0 ) );
    ss->addUniform( new osg::Uniform( "circleTextSampler", 0 ) );

    // Load shaders for rendering the circle highlight.
    _program = new osg::Program;
    _program->setName( "Circle shader" );

    osg::Shader* shader = new osg::Shader( osg::Shader::VERTEX );
    std::string shaderFileName( "circle.vs" );
    shaderFileName = osgDB::findDataFile( shaderFileName );
    if( shaderFileName.empty() )
    {
        osg::notify(osg::WARN) << "File \"" << shaderFileName << "\" not found." << std::endl;
        return;
    }
    shader->loadShaderSourceFromFile( shaderFileName );
    _program->addShader( shader );

    shader = new osg::Shader( osg::Shader::FRAGMENT );
    shaderFileName = "circle.fs";
    shaderFileName = osgDB::findDataFile( shaderFileName );
    if( shaderFileName.empty() )
    {
        osg::notify(osg::WARN) << "File \"" << shaderFileName << "\" not found." << std::endl;
        return;
    }
    shader->loadShaderSourceFromFile( shaderFileName );
    _program->addShader( shader );

    // HACK to work around a bug in OSG. Do not attach the program now, because
    // _labelGroup currently has nothing to draw. Attaching a program to such an
    // "empty" program causes the program to "leak" to other sibling nodes.
    //ss->setAttribute( _program );

    // default values for AutoTransform2
    ss->addUniform( new osg::Uniform( "at2_PivotPoint", osg::Vec3() ) );
    ss->addUniform( new osg::Uniform( "at2_Scale", 0.0f ) ); // Don't scale
}



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
    viewer.addEventHandler( ph );
    ph->setLabelText( "test label 01234" );

    viewer.realize();

    osg::notify( osg::ALWAYS ) << "Click on the model to highlight." << std::endl;
    osg::notify( osg::ALWAYS ) << "\tT/t\ttoggle highlights on/off." << std::endl;
    osg::notify( osg::ALWAYS ) << "\tDel\tClear all highlights." << std::endl;

    // main loop (note, window toolkits which take control over the main loop will require a window redraw callback containing the code below.)
    while(!viewer.done())
    {
        viewer.frame();
    } // while

    return 0;
} // main

