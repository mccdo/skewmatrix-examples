// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osg/Node>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgwTools/Shapes.h>
#include <osgwTools/Version.h>
#include <osg/OcclusionQueryNode>

#define USE_ISU_CB
#if defined( USE_ISU_CB )
#include "CameraImageCaptureCallback.h"
#else
#include <osgwTools/ScreenCapture.h>
#endif

#include <string>



//
//
//

#include <sstream>

unsigned int countGeometryVertices( osg::Geometry* geom )
{
    if (!geom->getVertexArray())
        return 0;

    // TBD This will eventually iterate over the PrimitiveSets and total the
    //   number of vertices actually used. But for now, it just returns the
    //   size of the vertex array.

    return geom->getVertexArray()->getNumElements();
}

class VertexCounter : public osg::NodeVisitor
{
public:
    VertexCounter( int limit )
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _limit( limit ),
        _total( 0 ) {}
    ~VertexCounter() {}

    int getTotal() { return _total; }
    bool exceeded() const { return _total > _limit; }
    void reset() { _total = 0; }

    virtual void apply( osg::Node& node )
    {
        // Check for early abort. If out total already exceeds the
        //   max number of vertices, no need to traverse further.
        if (exceeded())
            return;
        traverse( node );
    }

    virtual void apply( osg::Geode& geode )
    {
        // Possible early abort.
        if (exceeded())
            return;

        unsigned int i;
        for( i = 0; i < geode.getNumDrawables(); i++ )
        {
            osg::Geometry* geom = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
            if( !geom )
                continue;

            _total += countGeometryVertices( geom );

            if (_total > _limit)
                break;
        }
    }

protected:
    int _limit;
    int _total;
};


class OcclusionQueryVisitor : public osg::NodeVisitor
{
public:
    OcclusionQueryVisitor();
    virtual ~OcclusionQueryVisitor();

    // Specify the vertex count threshold for performing occlusion
    //   query tests. Nodes in the scene graph whose total child geometry
    //   contains fewer vertices than the specified threshold will
    //   never be tested, just drawn. (In fact, they will br treated as
    //   potential occluders and rendered first in front-to-back order.)
    void setOccluderThreshold( int vertices );
    int getOccluderThreshold() const;

    virtual void apply( osg::OcclusionQueryNode& oqn );
    virtual void apply( osg::Group& group );
    virtual void apply( osg::Geode& geode );

protected:
    void addOQN( osg::Node& node );

    // When an OQR creates all OQNs and each OQN shares the same OQC,
    //   these methods are used to uniquely name all OQNs. Handy
    //   for debugging.
    std::string getNextOQNName();
    int getNameIdx() const { return _nameIdx; }

    osg::ref_ptr<osg::StateSet> _state;
    osg::ref_ptr<osg::StateSet> _debugState;

    unsigned int _nameIdx;

    int _occluderThreshold;
};

OcclusionQueryVisitor::OcclusionQueryVisitor()
  : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
    _nameIdx( 0 ),
    _occluderThreshold( 5000 )
{
    // Create a dummy OcclusionQueryNode just so we can get its state.
    // We'll then share that state between all OQNs we add to the visited scene graph.
    osg::ref_ptr<osg::OcclusionQueryNode> oqn = new osg::OcclusionQueryNode;

    _state = oqn->getQueryStateSet();
    _debugState = oqn->getDebugStateSet();
}

OcclusionQueryVisitor::~OcclusionQueryVisitor()
{
    osg::notify( osg::INFO ) <<
        "osgOQ: OcclusionQueryVisitor: Added " << getNameIdx() <<
        " OQNodes." << std::endl;
}

void
OcclusionQueryVisitor::setOccluderThreshold( int vertices )
{
    _occluderThreshold = vertices;
}
int
OcclusionQueryVisitor::getOccluderThreshold() const
{
    return _occluderThreshold;
}

void
OcclusionQueryVisitor::apply( osg::OcclusionQueryNode& oqn )
{
    // A subgraph is already under osgOQ control.
    // Don't traverse further.
    return;
}

void
OcclusionQueryVisitor::apply( osg::Group& group )
{
    if (group.getNumParents() == 0)
    {
        // Can't add an OQN above a root node.
        traverse( group );
        return;
    }

    int preTraverseOQNCount = getNameIdx();
    traverse( group );

    if (getNameIdx() > preTraverseOQNCount)
        // A least one OQN was added below the current node.
        //   Don't add one here to avoid hierarchical nesting.
        return;

    // There are no OQNs below this group. If the vertex
    //   count exceeds the threshold, add an OQN here.
    addOQN( group );
}

void
OcclusionQueryVisitor::apply( osg::Geode& geode )
{
    if (geode.getNumParents() == 0)
    {
        // Can't add an OQN above a root node.
        traverse( geode );
        return;
    }

    addOQN( geode );
}

void
OcclusionQueryVisitor::addOQN( osg::Node& node )
{
    VertexCounter vc( _occluderThreshold );
    node.accept( vc );
    if (vc.exceeded())
    {
        // Insert OQN(s) above this node.
        unsigned int np = node.getNumParents();
        while (np--)
        {
            osg::Group* parent = dynamic_cast<osg::Group*>( node.getParent( np ) );
            if (parent != NULL)
            {
                osg::ref_ptr<osg::OcclusionQueryNode> oqn = new osg::OcclusionQueryNode();
                oqn->addChild( &node );
                parent->replaceChild( &node, oqn.get() );

                oqn->setName( getNextOQNName() );
                // Set all OQNs to use the same query StateSets (instead of multiple copies
                //   of the same StateSet) for efficiency.
                oqn->setQueryStateSet( _state.get() );
                oqn->setDebugStateSet( _debugState.get() );
            }
        }
    }
}

std::string
OcclusionQueryVisitor::getNextOQNName()
{
    std::ostringstream ostr;
    ostr << "OQNode_" << _nameIdx++;
    return ostr.str();
}

//
//
//



//
// Begin globalc
const int texW( 512 ), texH( 512 );

osg::ref_ptr< osg::Texture2D > tex;

osg::ref_ptr< osg::Camera > preRenderCamera;

#if defined( USE_ISU_CB )
std::string imageDumpName( "out.png" );
osg::ref_ptr< CameraImageCaptureCallback > captureCB;
#else
osg::ref_ptr< osgwTools::ScreenCapture > captureCB;
#endif

// End globals
//



class Handler : public osgGA::GUIEventHandler 
{
public: 
    Handler() : _frameCount( -1 ) {}
    ~Handler() {}

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        bool handled( false );
        switch(ea.getEventType())
        {
            case( osgGA::GUIEventAdapter::FRAME ):
            {
#if defined( USE_ISU_CB )
                if( _frameCount > -1 )
                {
                    if( --_frameCount == 0 )
                        preRenderCamera->setPostDrawCallback( NULL );
                }
#endif
            }
            case( osgGA::GUIEventAdapter::KEYDOWN ):
            {
                switch( ea.getKey() )
                {
                    case( osgGA::GUIEventAdapter::KEY_Return ):
                    {
#if defined( USE_ISU_CB )
                        preRenderCamera->setPostDrawCallback( captureCB.get() );
                        _frameCount = 2;
#else
                        captureCB->setNumFramesToCapture( 1 );
                        captureCB->setCapture( true );
#endif
                        handled = true;
                    }
                }
            }
        }
        return( handled );
    }

    int _frameCount;
};


osg::Node*
preRender( osg::Group* root, osg::Node* model )
{
    // Create the texture; we'll use this as our color buffer.
    // Note it has no image data; not required.
    tex = new osg::Texture2D;
    tex->setTextureWidth( texW );
    tex->setTextureHeight( texH );
    tex->setInternalFormat( GL_RGBA );
    tex->setBorderWidth( 0 );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
#if defined( USE_ISU_CB )
    osg::Image* image = new osg::Image;
    tex->setImage( image );
#endif


    // Configure preRenderCamera to draw fullscreen textured quad
    preRenderCamera = new osg::Camera;
    preRenderCamera->setClearColor( osg::Vec4( .5, 0., 0., 1. ) );

    preRenderCamera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    preRenderCamera->setRenderOrder( osg::Camera::PRE_RENDER );
    preRenderCamera->setViewMatrix( osg::Matrixd::translate( 0., 0., -30. ) );
    preRenderCamera->setProjectionMatrix( osg::Matrixd::perspective( 25., 1., 1., 100. ) );
    preRenderCamera->setViewport( new osg::Viewport( 0, 0, texW, texH ) );

    preRenderCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );
    preRenderCamera->attach( osg::Camera::COLOR_BUFFER0, tex.get(), 0, 0, false );

    preRenderCamera->addChild( model );
#if 1
    OcclusionQueryVisitor oqv;
    model->accept( oqv );
#endif

#if defined( USE_ISU_CB )
    captureCB = new CameraImageCaptureCallback( imageDumpName, texW, texH, tex.get() );
#else
    captureCB = new osgwTools::ScreenCapture;
    preRenderCamera->setPostDrawCallback( captureCB.get() );
#endif


    // Configure textured tri pair to hang off the root node and
    // display our texture.
    osg::Geode* geode( new osg::Geode );
    geode->addDrawable( osgwTools::makePlane(
        osg::Vec3( -1,0,-1 ), osg::Vec3( 2,0,0 ), osg::Vec3( 0,0,2 ) ) );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(
        0, tex.get(), osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    root->addChild( geode );


    return( preRenderCamera.get() );
}

int
main( int argc, char** argv )
{
    osg::ref_ptr< osg::Group > root( new osg::Group );
    osg::ref_ptr< osg::Node > model( osgDB::readNodeFile( "02-1100.ive" ) );
    if( !model.valid() )
        return( 1 );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    viewer.realize();
    viewer.addEventHandler( new Handler );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::ThreadingHandler );

    root->addChild( preRender( root.get(), model.get() ) );

    viewer.getCamera()->setClearColor( osg::Vec4( 0.4, 0.4, 0.4, 1. ) );

    return( viewer.run() );
}
