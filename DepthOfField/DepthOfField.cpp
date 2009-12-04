// Copyright (c) 2008 Skew Matrix Software LLC. All rights reserved.

#include <osg/Node>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/Stencil>
#include <osg/Depth>
#include <osg/CullFace>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#include <string>
#include <iostream>



class DepthOfField
{
public:
    DepthOfField();//default 1280x1024
    ~DepthOfField() {}

    // Change clear color, fov, and minZNear before calling this.
    void init( int argc, char** argv );

    // Call this once per frame.
    void update();

    // If you want to change the defaults, call these before init().
    void setClearColor( const osg::Vec4& c );
    void setFOV( float fov );
    void setMinZNear( float z );
    void setScene(osg::Node* scene);
    void setFocalPoint( float dist , float range);
    void setFocalDist(float dist);
    void setFocalRange(float range);
    void setWindowSize(float w, float h);

    float getFocalDist();
    float getFocalRange();


    osgViewer::Viewer* getViewer();

    osg::Camera* getViewerCamera();

    osg::Group* getSceneParent();
    osg::Program* createShader();
    

protected:
    void configureCamera(osg::Camera* camera, osg::Texture2D* texture, osg::Vec4 clearColor);
    void configureTexture(osg::Texture2D* texture);
    void setProjectionMatrices( const osg::Viewport* vp );

    osg::Geode* createSmallQuad( osg::Texture2D* colorTexture );
    osg::Geode* createDownsampleTexturedQuad( osg::Texture2D* colorTexture );
    osg::Geode* createFullscreenQuad(osg::Texture2D* colorTexture );
    osg::Geode* createBlurredXTexturedQuad( osg::Texture2D* colorTexture );
    osg::Geode* createBlurredYTexturedQuad( osg::Texture2D* colorTexture );
    osg::Geode* createDepthOfFieldQuad( osg::Texture2D* texture, osg::Texture2D* blurred );


    osg::ref_ptr< osgViewer::Viewer > _viewer;
    osg::ref_ptr< osg::Camera > _rttCamera;
    osg::ref_ptr< osg::Camera > _viewerCamera;
    osg::ref_ptr< osg::Camera > _rtsqCamera;
    osg::ref_ptr< osg::Camera > _rtsCamera;
    osg::ref_ptr< osg::Camera > _rtbxCamera;
    osg::ref_ptr< osg::Camera > _rtbyCamera;
    osg::ref_ptr< osg::Camera > _rtdofCamera;
    osg::ref_ptr< osg::Group > _parent;
    osg::ref_ptr< osg::Node > _scene;

    osg::ref_ptr< osgGA::TrackballManipulator > _manipulator;
    osg::ref_ptr< osg::Viewport > _lastViewport;

    osg::ref_ptr< osg::Texture2D > _texMap;
    osg::ref_ptr< osg::Texture2D > _smallMap;
    osg::ref_ptr< osg::Texture2D > _smallQuadMap;
    osg::ref_ptr< osg::Texture2D > _blurxMap;
    osg::ref_ptr< osg::Texture2D > _bluryMap;
    osg::ref_ptr< osg::Texture2D > _dofMap;

    osg::ref_ptr< osg::Uniform > _focalDist;
    osg::ref_ptr< osg::Uniform > _focalRange;

    osg::Vec4 _clearColor;
    float _fov;
    //float _focalDist;
    //float _focalRange;

    // This is the max dimensions of the RTT surface. Expanding the window size
    // beyond these dimensions will show some of the viewer camera's clear color.
    // This is bad and is a limitation of this code in its current form.
    float _maxWidth;
    float _maxHeight;
};
class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:

    KeyboardEventHandler(DepthOfField* dof)
    {_dof = dof;}


    
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
    {
        float f;
        switch (ea.getEventType())
        {
        case(osgGA::GUIEventAdapter::KEYDOWN):
            {
                switch (ea.getKey())
                {
                // '+'/'-': Change the focal distance. 
                // ('=' is synonymous with '+')
                case '=':
                case '+':
                    f = _dof->getFocalDist();
                    f = f + 5;
                    _dof->setFocalDist(f);
                    return true;
                case '-':
                    f = _dof->getFocalDist();
                    f = f - 5;
                    _dof->setFocalDist(f);
                    return true;

                // '['/']': Expand/contact focal range.
                case '[':
                    f = _dof->getFocalRange();
                    f = f * 4.0/3.0;
                    _dof->setFocalRange(f);
                    return true;
                case ']':
                    f = _dof->getFocalRange();
                    f = f * 3.0/4.0;
                    _dof->setFocalRange(f);
                    return true;
                }
            }
        default:
            break;

        }
        //return false to allow mouse manipulation
        return false;
    }
private:
    DepthOfField* _dof;
};

DepthOfField::DepthOfField()
  : _clearColor( osg::Vec4( .2, .2, .4, 1. ) ),
    _fov( 50.f ),
    _maxWidth( 1280 ),
    _maxHeight( 1024 )
{
    _focalDist = new osg::Uniform("focalDist", float( -40.0));
    _focalRange = new osg::Uniform("focalRange", float( 20.0));
}

void DepthOfField::setScene(osg::Node* scene)
{
    _scene = scene;
}

void DepthOfField::init( int argc, char** argv )
{
 
    _viewer = new osgViewer::Viewer;
    _viewer->setUpViewInWindow( 10, 30, _maxWidth, _maxHeight);

    // Create and configure the textures.
    // We do 5 render passes, but one might be able to be removed in
    // the future:
    //   1. Render to a full-sized texture. This is a "focused image".
    //   2. Render to a small texture. This will be the "blurry image".
    //   3. Blur the small texture in x.
    //   4. Blut the x-blurred texture in y.
    //   5. Combine the y-blurred texture and focused texture based
    //      on each pixel's depth/distance value.
    _texMap       = new osg::Texture2D();
    _smallQuadMap = new osg::Texture2D();
    _smallMap     = new osg::Texture2D();
    _blurxMap     = new osg::Texture2D();
    _bluryMap     = new osg::Texture2D();
    _dofMap     = new osg::Texture2D();
    configureTexture( _dofMap.get() );
    configureTexture( _texMap.get() );
    configureTexture( _smallQuadMap.get() );
    configureTexture( _smallMap.get() );
    configureTexture( _blurxMap.get() );
    configureTexture( _bluryMap.get() );
    
    _rttCamera  = new osg::Camera;
    _rtsqCamera = new osg::Camera;
    _rtsCamera  = new osg::Camera;
    _rtbxCamera = new osg::Camera;
    _rtbyCamera = new osg::Camera;
    _rtdofCamera = new osg::Camera;
  
    configureCamera(_rttCamera.get()  ,_texMap.get(),       _clearColor                 );
    configureCamera(_rtsqCamera.get() ,_smallQuadMap.get(), osg::Vec4( 1. , .0, .0, 1. ));
    configureCamera(_rtsCamera.get()  ,_smallMap.get(),     osg::Vec4( .0 , .0, 1., 1. ));
    configureCamera(_rtbxCamera.get() ,_blurxMap.get(),     osg::Vec4( 1.0, .5, .0, 1. ));
    configureCamera(_rtbyCamera.get() ,_bluryMap.get(),     osg::Vec4( 1.0, .0, .5, 1. ));
    configureCamera(_rtdofCamera.get(),_dofMap.get(),       osg::Vec4( .0 , 1., .5, 1. ));

    _viewerCamera = _viewer->getCamera();
    _viewerCamera->setViewMatrix( osg::Matrix::identity() );
    _viewerCamera->setClearColor( osg::Vec4( 0., 1., 0., 1. ) );
    

    // Create the scene parent. The application can add and remove scene data to/from this node.
    _parent = new osg::Group;
    _parent->getOrCreateStateSet()->addUniform( _focalDist.get() );
    _parent->getOrCreateStateSet()->addUniform( _focalRange.get() );
    
    _rttCamera->addChild( _parent.get() );

    {
        // Render the quad in eye space. Use an ABSOLUTE_RF MatrixTransform and leave its matrix as identity.
        osg::MatrixTransform* eyeSpace = new osg::MatrixTransform;
        eyeSpace->setReferenceFrame( osg::Transform::ABSOLUTE_RF );

        // Here's how we handle lighting. To mimic the osgViewer's built-in headlight, we
        // position GL_LIGHT0 in eye space at the viewer location. Since this is an infinite
        // light, the position vector is a (eye space) direction; in this case +Z.
        osg::ref_ptr<osg::Light> light = new osg::Light;
        light->setLightNum( 0 );
        light->setDiffuse( osg::Vec4( 1., 1., 1., 1. ) );
        light->setSpecular( osg::Vec4( 1., 1., 1., 1. ) );
        light->setPosition( osg::Vec4( 0., 0., 1., 0. ) );

        osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
        ls->setLight( light.get() );
        eyeSpace->addChild( ls.get() );
        _parent->addChild( eyeSpace );
    }
    if( _scene.valid() )
    {
        _parent->addChild( _scene.get() );
    }
    else
        osg::notify( osg::FATAL ) << "No scene data. Exiting." << std::endl;

    // Heirarchically arrange all cameras and quad geometry
    osg::ref_ptr< osg::Group > smallQ = new osg::Group();
    smallQ->addChild( _rttCamera.get() );
    smallQ->addChild( createSmallQuad( _texMap.get() ) );
    _rtsqCamera->addChild(smallQ.get());

    osg::ref_ptr< osg::Group > small = new osg::Group();
    small->addChild(_rtsqCamera.get() );
    small->addChild( createFullscreenQuad(_smallQuadMap.get() ) );
    _rtsCamera->addChild(small.get() );

    osg::ref_ptr< osg::Group > blurX = new osg::Group();
    blurX->addChild(_rtsCamera.get() );
    blurX->addChild(createDownsampleTexturedQuad( _smallMap.get() ) );
    _rtbxCamera->addChild( blurX.get() );

    osg::ref_ptr< osg::Group > blurY = new osg::Group();
    blurY->addChild(_rtbxCamera.get());
    blurY->addChild(createBlurredXTexturedQuad(_blurxMap.get()));
    _rtbyCamera->addChild( blurY.get() );

    osg::ref_ptr< osg::Group > dofg = new osg::Group();
    dofg->addChild(_rtbyCamera.get());
    dofg->addChild(createBlurredYTexturedQuad( _bluryMap.get() ) );
    _rtdofCamera->addChild(dofg.get());

    osg::ref_ptr< osg::Group > root = new osg::Group();
    root->addChild(_rtdofCamera.get());
    root->addChild(createDepthOfFieldQuad( _texMap.get(),_dofMap.get() ) );
    _viewer->setSceneData( root.get() );


    // To manipulate the scene beneath the RTT Camera, attach the TB manipulator
    // as a viewer event handler. In the update() function, called once per frame,
    // set the RTT Camera with the manipulator's inverse matrix.
    _manipulator = new osgGA::TrackballManipulator;
    _viewer->addEventHandler( _manipulator.get() );

    // We want the home position (space bar) to be based on the elements of our
    // scene -- just the stuff under _parent.
    _manipulator->setNode( _parent.get() );
    _manipulator->home( 0. );

    // We need to look for changes in the Viewport so we can reconfigure both the
    // viewer camera's and the RTT camera's projection matrices accordingly.
    // Init the viewer, get its viewport, and set the initial projection matrices
    // so that the first frame is correct.
    _viewer->realize();
    _lastViewport = new osg::Viewport( *( getViewerCamera()->getViewport() ) );
    setProjectionMatrices( getViewerCamera()->getViewport() );
}

// This method must be called once per frame. It is responsible for modifying the RTT
// camera position based on the TB manipulator, and also checking for (and handling)
// changes to the window size / viewport. Note that we could probably catch window
// resize / viewport changes with an event handler, which might actually be more
// efficient than doing a viewport compare. Either solution is acceptable.
void DepthOfField::update()
{
    // Set the RTT camera view from the manipulator.
    _rttCamera->setViewMatrix( _manipulator->getInverseMatrix() );

    // TBD We can remove support for viewport changes, as windowed
    // rendering isn't a requirement for ISU.
    //
    // Update the projection matrices if the viewport changed.
    osg::Viewport* currentViewport = getViewerCamera()->getViewport();
    if (*_lastViewport != *currentViewport)
    {
        setProjectionMatrices( currentViewport );
        _lastViewport = new osg::Viewport( *currentViewport );
    }
}

// This is the clear color of the scene.
// If you want to change the default, call this before init().
void DepthOfField::setClearColor( const osg::Vec4& c )
{
    _clearColor = c;
}

// Field of view, default is 50.
// If you want to change the default, call this before init().
void DepthOfField::setFOV( float fov )
{
    _fov = fov;
}
void DepthOfField::setFocalPoint( float dist, float range )
{
    setFocalDist(dist);
    setFocalRange(range);
}
void DepthOfField::setFocalDist( float dist)
{
    _focalDist->set(dist);
}
void DepthOfField::setFocalRange(float range)
{
     _focalRange->set(range);
}
void DepthOfField::setWindowSize(float w, float h )
{
    _maxWidth = w;
    _maxHeight = h;
}
float DepthOfField::getFocalDist()
{
    float dist;
    _focalDist->get(dist);
    return dist;
}
float DepthOfField::getFocalRange()
{
    float range;
    _focalRange->get(range);
    return range;
}
// Convenience accessor in case app needs to get to the viewer.
osgViewer::Viewer* DepthOfField::getViewer()
{
    return _viewer.get();
}

// Convenience accessor and equivalent to getViewer()->getCamera().
osg::Camera* DepthOfField::getViewerCamera()
{
    return _viewer->getCamera();
}

osg::Group* DepthOfField::getSceneParent()
{
    return _parent.get();
}


void DepthOfField::configureCamera(osg::Camera* camera, osg::Texture2D* texture, osg::Vec4 clearColor)
{
    camera->setRenderOrder( osg::Camera::PRE_RENDER );
    camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    camera->setViewMatrix( osg::Matrix::identity() );
    camera->setClearMask( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    camera->setClearColor( clearColor );
    camera->attach( osg::Camera::COLOR_BUFFER, texture );
    
}

// Configure the textures that the RTT camera will render into.
void DepthOfField::configureTexture(osg::Texture2D* texture)
{
    texture->setInternalFormat( GL_RGBA );
    texture->setTextureSize( _maxWidth, _maxHeight );
    texture->setSourceFormat( GL_RGBA );
    texture->setSourceType( GL_UNSIGNED_BYTE );
    texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE );
    texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
}

// Set the camera's projection matrices. Must be called once before the first frame (when the
// window and viewport are known), and again if the viewport changes.
void DepthOfField::setProjectionMatrices(const osg::Viewport* vp )
{
    // Configure the top-level viewer camera, which merely renders a screen-oriented quad with a texture
    // (to display the final image). The modelview martix for this camera is always identity, so we're always
    // rendering in eye space. Set the projection matrix to control the amount of eye space mapped into clip
    // space. In other workd, configure left, right, bottom, and top so that the view volume only contains
    // the portion of the quad that shows the region of the texture that was rendered into during the RTT pass.
    const float x = vp->x();
    const float y = vp->y();
    const float width = vp->width();
    const float height = vp->height();
    float left = x;
    float right = x+width;
    float bottom = y;
    float top = y+height;
    _viewerCamera->setProjectionMatrixAsOrtho(left,right,bottom,top, -1, 1 );
    _rtdofCamera->setProjectionMatrixAsOrtho(left,right,bottom,top, -1, 1 );
    _rtbyCamera->setProjectionMatrixAsOrtho(left,right,bottom,top, -1, 1 );
    _rtbxCamera->setProjectionMatrixAsOrtho(left,right,bottom,top, -1, 1 );
    _rtsCamera->setProjectionMatrixAsOrtho(left,right,bottom,top, -1, 1 );
    _rtsqCamera->setProjectionMatrixAsOrtho(left,right,bottom,top, -1, 1 );

    // Set RTT camera projection to account for aspect ratio. Near/far are ignored when
    // the camera is configured with default auto compute.
    _rttCamera->setProjectionMatrixAsPerspective( _fov, (width/height), .1, 100. );
}


// Each render pass (after initially rendering the scene) is just
// a textured quad. TBD: Probably only need one of these functions
// and the differences such as sizes and StateSets could be handled
// by parameters. But for now, we have a specialized function for
// creating each quad in the processing pipeline.
osg::Geode* DepthOfField::createSmallQuad( osg::Texture2D* colorTexture )
{
    // The geometry is eye space (the viewer's camera has an identity modelview).
    // If the window is sized to _maxWidth x _maxHeight, then the entire -1 to 1
    // quad will be visible. When the window is smaller, the viewer camera's projection matrix is
    // modified (see setProjectionMatrices) to display the lower left corner of the quad.
    double xMin = 0;
    double xMax = _maxWidth/2;
    double yMin = 0;
    double yMax = _maxHeight/2;
    double zVal = 0;
    
    osg::Geode* quadGeode = new osg::Geode();
    osg::ref_ptr< osg::Geometry > quadGeometry = new osg::Geometry();

    osg::ref_ptr< osg::Vec3Array > quadVertices = new osg::Vec3Array();
    quadVertices->resize( 4 );
    (*quadVertices)[ 0 ].set( xMin, yMin, zVal );
    (*quadVertices)[ 1 ].set( xMax, yMin, zVal );
    (*quadVertices)[ 2 ].set( xMax, yMax, zVal );
    (*quadVertices)[ 3 ].set( xMin, yMax, zVal );
    quadGeometry->setVertexArray( quadVertices.get() );
    
    osg::ref_ptr< osg::Vec2Array > quadTexCoords = new osg::Vec2Array();
    quadTexCoords->resize( 4 );
    (*quadTexCoords)[ 0 ].set( 0, 0 );
    (*quadTexCoords)[ 1 ].set( 1, 0 );
    (*quadTexCoords)[ 2 ].set( 1, 1 );
    (*quadTexCoords)[ 3 ].set( 0, 1 );
    quadGeometry->setTexCoordArray( 0, quadTexCoords.get() );

    osg::ref_ptr< osg::Vec4Array > color = new osg::Vec4Array();
    color->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    quadGeometry->setColorArray( color.get() );
    quadGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );

    quadGeometry->addPrimitiveSet( new osg::DrawArrays(
        osg::PrimitiveSet::QUADS, 0, 4 ) );
    
    quadGeode->addDrawable( quadGeometry.get() );
    quadGeode->setCullingActive( false );


    osg::ref_ptr< osg::StateSet > stateset = new osg::StateSet();
    // Don't light this quad.
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    // Texture in unit 0.
    stateset->setTextureAttributeAndModes(
        0, colorTexture, osg::StateAttribute::ON );

    quadGeode->setStateSet( stateset.get() );

    return quadGeode;
}

osg::Geode* DepthOfField::createDownsampleTexturedQuad( osg::Texture2D* colorTexture )
{
    double xMin = 0;
    double xMax = _maxWidth;
    double yMin = 0;
    double yMax = _maxHeight;
    double zVal = 0;
    
    osg::Geode* quadGeode = new osg::Geode();
    osg::ref_ptr< osg::Geometry > quadGeometry = new osg::Geometry();

    osg::ref_ptr< osg::Vec3Array > quadVertices = new osg::Vec3Array();
    quadVertices->resize( 4 );
    (*quadVertices)[ 0 ].set( xMin, yMin, zVal );
    (*quadVertices)[ 1 ].set( xMax, yMin, zVal );
    (*quadVertices)[ 2 ].set( xMax, yMax, zVal );
    (*quadVertices)[ 3 ].set( xMin, yMax, zVal );
    quadGeometry->setVertexArray( quadVertices.get() );
    
    osg::ref_ptr< osg::Vec2Array > quadTexCoords = new osg::Vec2Array();
    quadTexCoords->resize( 4 );
    (*quadTexCoords)[ 0 ].set( 0, 0 );
    (*quadTexCoords)[ 1 ].set( .5, 0 );
    (*quadTexCoords)[ 2 ].set( .5, .5 );
    (*quadTexCoords)[ 3 ].set( 0, .5 );
    quadGeometry->setTexCoordArray( 0, quadTexCoords.get() );

    osg::ref_ptr< osg::Vec4Array > color = new osg::Vec4Array();
    color->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    quadGeometry->setColorArray( color.get() );
    quadGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );

    quadGeometry->addPrimitiveSet( new osg::DrawArrays(
        osg::PrimitiveSet::QUADS, 0, 4 ) );
    
    quadGeode->addDrawable( quadGeometry.get() );
    quadGeode->setCullingActive( false );

    osg::ref_ptr< osg::StateSet > stateset = new osg::StateSet();
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateset->setTextureAttributeAndModes(
        0, colorTexture, osg::StateAttribute::ON );
    
    quadGeode->setStateSet( stateset.get() );

    return quadGeode;
}
osg::Geode* DepthOfField::createFullscreenQuad( osg::Texture2D* colorTexture )
{
    double xMin = 0;
    double xMax = _maxWidth;
    double yMin = 0;
    double yMax = _maxHeight;
    double zVal = 0;
    
    osg::Geode* quadGeode = new osg::Geode();
    osg::ref_ptr< osg::Geometry > quadGeometry = new osg::Geometry();

    osg::ref_ptr< osg::Vec3Array > quadVertices = new osg::Vec3Array();
    quadVertices->resize( 4 );
    (*quadVertices)[ 0 ].set( xMin, yMin, zVal );
    (*quadVertices)[ 1 ].set( xMax, yMin, zVal );
    (*quadVertices)[ 2 ].set( xMax, yMax, zVal );
    (*quadVertices)[ 3 ].set( xMin, yMax, zVal );
    quadGeometry->setVertexArray( quadVertices.get() );
    
    osg::ref_ptr< osg::Vec2Array > quadTexCoords = new osg::Vec2Array();
    quadTexCoords->resize( 4 );
    (*quadTexCoords)[ 0 ].set( 0, 0 );
    (*quadTexCoords)[ 1 ].set( 1, 0 );
    (*quadTexCoords)[ 2 ].set( 1, 1 );
    (*quadTexCoords)[ 3 ].set( 0, 1 );
    quadGeometry->setTexCoordArray( 0, quadTexCoords.get() );

    osg::ref_ptr< osg::Vec4Array > color = new osg::Vec4Array();
    color->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    quadGeometry->setColorArray( color.get() );
    quadGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );

    quadGeometry->addPrimitiveSet( new osg::DrawArrays(
        osg::PrimitiveSet::QUADS, 0, 4 ) );
    
    quadGeode->addDrawable( quadGeometry.get() );
    quadGeode->setCullingActive( false );

    osg::ref_ptr< osg::StateSet > stateset = new osg::StateSet();
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateset->setTextureAttributeAndModes(
        0, colorTexture, osg::StateAttribute::ON );
    
    quadGeode->setStateSet( stateset.get() );

    return quadGeode;
}

osg::Geode* DepthOfField::createBlurredXTexturedQuad( osg::Texture2D* colorTexture )
{
    double xMin = 0;
    double xMax = _maxWidth;
    double yMin = 0;
    double yMax = _maxHeight;
    double zVal = 0;
    
    osg::Geode* quadGeode = new osg::Geode();
    osg::ref_ptr< osg::Geometry > quadGeometry = new osg::Geometry();

    osg::ref_ptr< osg::Vec3Array > quadVertices = new osg::Vec3Array();
    quadVertices->resize( 4 );
    (*quadVertices)[ 0 ].set( xMin, yMin, zVal );
    (*quadVertices)[ 1 ].set( xMax, yMin, zVal );
    (*quadVertices)[ 2 ].set( xMax, yMax, zVal );
    (*quadVertices)[ 3 ].set( xMin, yMax, zVal );
    quadGeometry->setVertexArray( quadVertices.get() );
    
    osg::ref_ptr< osg::Vec2Array > quadTexCoords = new osg::Vec2Array();
    quadTexCoords->resize( 4 );
    (*quadTexCoords)[ 0 ].set( 0, 0 );
    (*quadTexCoords)[ 1 ].set( 1, 0 );
    (*quadTexCoords)[ 2 ].set( 1, 1 );
    (*quadTexCoords)[ 3 ].set( 0, 1 );
    quadGeometry->setTexCoordArray( 0, quadTexCoords.get() );

    osg::ref_ptr< osg::Vec4Array > color = new osg::Vec4Array();
    color->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    quadGeometry->setColorArray( color.get() );
    quadGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );

    quadGeometry->addPrimitiveSet( new osg::DrawArrays(
        osg::PrimitiveSet::QUADS, 0, 4 ) );
    
    quadGeode->addDrawable( quadGeometry.get() );
    quadGeode->setCullingActive( false );

    osg::ref_ptr< osg::StateSet > stateset = new osg::StateSet();
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateset->setTextureAttributeAndModes(
        0, colorTexture, osg::StateAttribute::ON );

    float dx = 1/_maxWidth;
    osg::Uniform* horzTapOffs = new osg::Uniform();
    horzTapOffs->setName("offset");
    horzTapOffs->setNumElements(6);
    horzTapOffs->setType(osg::Uniform::FLOAT_VEC4);
    horzTapOffs->setElement(0, osg::Vec4(1.3366 * dx,0.,0.,0.));
    horzTapOffs->setElement(1, osg::Vec4(3.4295 * dx,0.,0.,0.));
    horzTapOffs->setElement(2, osg::Vec4(5.4264 * dx,0.,0.,0.));
    horzTapOffs->setElement(3, osg::Vec4(7.4395 * dx,0.,0.,0.));
    horzTapOffs->setElement(4, osg::Vec4(9.4436 * dx,0.,0.,0.));
    horzTapOffs->setElement(5, osg::Vec4(11.4401* dx,0.,0.,0.));
    stateset->addUniform(horzTapOffs);
    stateset->addUniform(new osg::Uniform("sample",0));
    
    std::string vertsource =
        "uniform vec4 offset[6]; \n"
        "void main() \n"
            "{ \n"
                "gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; \n"
                "gl_TexCoord[0] = gl_MultiTexCoord0; \n"
                "gl_TexCoord[1] = gl_MultiTexCoord0 + offset[0]; \n"
                "gl_TexCoord[2] = gl_MultiTexCoord0 + offset[1]; \n"
                "gl_TexCoord[3] = gl_MultiTexCoord0 + offset[2]; \n"
                "gl_TexCoord[4] = gl_MultiTexCoord0 - offset[0]; \n"
                "gl_TexCoord[5] = gl_MultiTexCoord0 - offset[1]; \n"
                "gl_TexCoord[6] = gl_MultiTexCoord0 - offset[2]; \n"
            "} \n";

    // Fragment shader for blurring in X is very confusing.
    // Sample points are modified based on distance from the eye.
    // Code is translated directly from the ATI HLSL code taken
    // from the Shader X 2 article.
    std::string fragsource = 
        "uniform sampler2D sample; \n"
        "uniform vec4 offset[6]; \n"
        "void main() \n"
        "{ \n"
        
            "vec4 s0,s1,s2,s3,s4,s5,s6; \n"
            "vec4 vWeights4; \n"
            "vec3 vWeights3; \n"
            "vec3 vColorSum; \n"
            "float fWeightSum; \n"
            "vec4 vThresh0 = vec4(0.1,0.3,0.5,-0.01); \n"
            "vec4 vThresh1 = vec4(0.6,0.7,0.8,0.9); \n"
            "s0 = texture2D(sample, gl_TexCoord[0].xy); \n"
            "s1 = texture2D(sample, gl_TexCoord[1].xy); \n"
            "s2 = texture2D(sample, gl_TexCoord[2].xy); \n"
            "s3 = texture2D(sample, gl_TexCoord[3].xy); \n"
            "s4 = texture2D(sample, gl_TexCoord[4].xy); \n"
            "s5 = texture2D(sample, gl_TexCoord[5].xy); \n"
            "s6 = texture2D(sample, gl_TexCoord[6].xy); \n"
            
            "vWeights4.x = clamp(s1.a-vThresh0.x,0.0,1.0); \n"
            "vWeights4.y = clamp(s2.a-vThresh0.y,0.0,1.0); \n"
            "vWeights4.z = clamp(s3.a-vThresh0.z,0.0,1.0); \n"
            "vWeights4.w = clamp(s0.a-vThresh0.w,0.0,1.0); \n"

            "vColorSum = s0.rgb*vWeights4.x + s1.rgb*vWeights4.y+ s2.rgb*vWeights4.z+ s3.rgb*vWeights4.w; \n"

            "fWeightSum = dot(vWeights4,vec4(1,1,1,1)); \n"

            "vWeights3.x = clamp(s4.a-vThresh0.x,0.0,1.0); \n"
            "vWeights3.y = clamp(s5.a-vThresh0.y,0.0,1.0); \n"
            "vWeights3.z = clamp(s6.a-vThresh0.z,0.0,1.0); \n"

            "vColorSum += s4.rgb*vWeights3.x + s5.rgb*vWeights3.y+ s6.rgb*vWeights3.z; \n"

            "fWeightSum += dot(vWeights3,vec3(1,1,1)); \n"

            "s0 = texture2D(sample, gl_TexCoord[0].xy+offset[3]); \n"
            "s1 = texture2D(sample, gl_TexCoord[0].xy+offset[4]); \n"
            "s2 = texture2D(sample, gl_TexCoord[0].xy+offset[5]); \n"
            "s3 = texture2D(sample, gl_TexCoord[0].xy-offset[3]); \n"
            "s4 = texture2D(sample, gl_TexCoord[0].xy-offset[4]); \n"
            "s5 = texture2D(sample, gl_TexCoord[0].xy-offset[5]); \n"
            
            "vWeights3.x = clamp(s0.a-vThresh1.x,0.0,1.0); \n"
            "vWeights3.y = clamp(s1.a-vThresh1.y,0.0,1.0); \n"
            "vWeights3.z = clamp(s2.a-vThresh1.z,0.0,1.0); \n"

            "vColorSum += s0.rgb*vWeights3.x + s1.rgb*vWeights3.y+ s2.rgb*vWeights3.z; \n"

            "fWeightSum += dot(vWeights3,vec3(1,1,1)); \n"

            "vWeights3.x = clamp(s3.a-vThresh1.x,0.0,1.0); \n"
            "vWeights3.y = clamp(s4.a-vThresh1.y,0.0,1.0); \n"
            "vWeights3.z = clamp(s5.a-vThresh1.z,0.0,1.0); \n"

            "vColorSum += s3.rgb*vWeights3.x + s4.rgb*vWeights3.y+ s5.rgb*vWeights3.z; \n"

            "fWeightSum += dot(vWeights3,vec3(1,1,1)); \n"



            "vColorSum = vColorSum / fWeightSum; \n"

            "gl_FragColor = vec4(vColorSum,fWeightSum);//*0.00390625; \n"

        "} \n";
    osg::Shader* vertShader = new osg::Shader();
    vertShader->setType(osg::Shader::VERTEX);
    vertShader->setShaderSource(vertsource);

    osg::Shader* fragShader = new osg::Shader();
    fragShader->setType(osg::Shader::FRAGMENT);
    fragShader->setShaderSource(fragsource);

    osg::Program* program = new osg::Program();
    program->addShader(vertShader);
    program->addShader(fragShader);

    stateset->setAttribute(program,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    quadGeode->setStateSet( stateset.get() );

    return quadGeode;
}
osg::Geode* DepthOfField::createBlurredYTexturedQuad( osg::Texture2D* colorTexture )
{
    double xMin = 0;
    double xMax = _maxWidth;
    double yMin = 0;
    double yMax = _maxHeight;
    double zVal = 0;
    
    osg::Geode* quadGeode = new osg::Geode();
    osg::ref_ptr< osg::Geometry > quadGeometry = new osg::Geometry();

    osg::ref_ptr< osg::Vec3Array > quadVertices = new osg::Vec3Array();
    quadVertices->resize( 4 );
    (*quadVertices)[ 0 ].set( xMin, yMin, zVal );
    (*quadVertices)[ 1 ].set( xMax, yMin, zVal );
    (*quadVertices)[ 2 ].set( xMax, yMax, zVal );
    (*quadVertices)[ 3 ].set( xMin, yMax, zVal );
    quadGeometry->setVertexArray( quadVertices.get() );
    
    osg::ref_ptr< osg::Vec2Array > quadTexCoords = new osg::Vec2Array();
    quadTexCoords->resize( 4 );
    (*quadTexCoords)[ 0 ].set( 0, 0 );
    (*quadTexCoords)[ 1 ].set( 1, 0 );
    (*quadTexCoords)[ 2 ].set( 1, 1 );
    (*quadTexCoords)[ 3 ].set( 0, 1 );
    quadGeometry->setTexCoordArray( 0, quadTexCoords.get() );

    osg::ref_ptr< osg::Vec4Array > color = new osg::Vec4Array();
    color->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    quadGeometry->setColorArray( color.get() );
    quadGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );

    quadGeometry->addPrimitiveSet( new osg::DrawArrays(
        osg::PrimitiveSet::QUADS, 0, 4 ) );
    
    quadGeode->addDrawable( quadGeometry.get() );
    quadGeode->setCullingActive( false );

    osg::ref_ptr< osg::StateSet > stateset = new osg::StateSet();
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateset->setTextureAttributeAndModes(
        0, colorTexture, osg::StateAttribute::ON );

    float dy = 1/_maxHeight;
    osg::Uniform* vertTapOffs = new osg::Uniform();
    vertTapOffs->setName("offset");
    vertTapOffs->setNumElements(6);
    vertTapOffs->setType(osg::Uniform::FLOAT_VEC4);
    vertTapOffs->setElement(0, osg::Vec4(0.,1.3366 * dy,0.,0.));
    vertTapOffs->setElement(1, osg::Vec4(0.,3.4295 * dy,0.,0.));
    vertTapOffs->setElement(2, osg::Vec4(0.,5.4264 * dy,0.,0.));
    vertTapOffs->setElement(3, osg::Vec4(0.,7.4395 * dy,0.,0.));
    vertTapOffs->setElement(4, osg::Vec4(0.,9.4436 * dy,0.,0.));
    vertTapOffs->setElement(5, osg::Vec4(0.,11.4401* dy,0.,0.));
    stateset->addUniform(vertTapOffs);
    stateset->addUniform(new osg::Uniform("sample",0));
    
    std::string vertsource =
        "uniform vec4 offset[6]; \n"
        "void main() \n"
            "{ \n"
                "gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; \n"
                "gl_TexCoord[0] = gl_MultiTexCoord0; \n"
                "gl_TexCoord[1] = gl_MultiTexCoord0 + offset[0]; \n"
                "gl_TexCoord[2] = gl_MultiTexCoord0 + offset[1]; \n"
                "gl_TexCoord[3] = gl_MultiTexCoord0 + offset[2]; \n"
                "gl_TexCoord[4] = gl_MultiTexCoord0 - offset[0]; \n"
                "gl_TexCoord[5] = gl_MultiTexCoord0 - offset[1]; \n"
                "gl_TexCoord[6] = gl_MultiTexCoord0 - offset[2]; \n"
            "} \n";

    // Fragmnt shader for blurring in Y is less complex than blurring in X,
    // but still just translated directly from HLSL, so it's pretty ugly.
    std::string fragsource = 
        "uniform sampler2D sample; \n"
        "uniform vec4 offset[6]; \n"
        "void main() \n"
        "{ \n"
        
            "vec4 s0,s1,s2,s3,s4,s5,s6; \n"
            "vec4 vWeights0 = vec4(0.080,0.075,0.070,0.100); \n"
            "vec4 vWeights1 = vec4(0.065,0.060,0.055,0.050); \n"
            "vec4 vColorWeightSum; \n"
            "float fWeightSum; \n"
            "vec4 vThresh0 = vec4(0.1,0.3,0.5,-0.01); \n"
            "vec4 vThresh1 = vec4(0.6,0.7,0.8,0.9); \n"
            "s0 = texture2D(sample, gl_TexCoord[0].xy); \n"
            "s1 = texture2D(sample, gl_TexCoord[1].xy); \n"
            "s2 = texture2D(sample, gl_TexCoord[2].xy); \n"
            "s3 = texture2D(sample, gl_TexCoord[3].xy); \n"
            "s4 = texture2D(sample, gl_TexCoord[4].xy); \n"
            "s5 = texture2D(sample, gl_TexCoord[5].xy); \n"
            "s6 = texture2D(sample, gl_TexCoord[6].xy); \n"
            
            "s0.rgb = s0.rgb * s0.a; \n"
            "s1.rgb = s1.rgb * s1.a; \n"
            "s2.rgb = s2.rgb * s2.a; \n"
            "s3.rgb = s3.rgb * s3.a; \n"
            "s4.rgb = s4.rgb * s4.a; \n"
            "s5.rgb = s5.rgb * s5.a; \n"
            "s6.rgb = s6.rgb * s6.a; \n"


            "vColorWeightSum = s0 * vWeights0.w + (s1+s4)*vWeights0.x + (s2+s5)*vWeights0.y + (s3+s6)*vWeights0.z; \n"

            "s0 = texture2D(sample, gl_TexCoord[0].xy+offset[3]); \n"
            "s1 = texture2D(sample, gl_TexCoord[0].xy+offset[4]); \n"
            "s2 = texture2D(sample, gl_TexCoord[0].xy+offset[5]); \n"
            "s3 = texture2D(sample, gl_TexCoord[0].xy-offset[3]); \n"
            "s4 = texture2D(sample, gl_TexCoord[0].xy-offset[4]); \n"
            "s5 = texture2D(sample, gl_TexCoord[0].xy-offset[5]); \n"
            
            "s0.rgb = s0.rgb * s0.a; \n"
            "s1.rgb = s1.rgb * s1.a; \n"
            "s2.rgb = s2.rgb * s2.a; \n"
            "s3.rgb = s3.rgb * s3.a; \n"
            "s4.rgb = s4.rgb * s4.a; \n"
            "s5.rgb = s5.rgb * s5.a; \n"

           
            "vColorWeightSum.rgb = vColorWeightSum.rgb / vColorWeightSum.a; \n"

            "gl_FragColor = vColorWeightSum;//*256.0; \n"

        "} \n";
    osg::Shader* vertShader = new osg::Shader();
    vertShader->setType(osg::Shader::VERTEX);
    vertShader->setShaderSource(vertsource);

    osg::Shader* fragShader = new osg::Shader();
    fragShader->setType(osg::Shader::FRAGMENT);
    fragShader->setShaderSource(fragsource);

    osg::Program* program = new osg::Program();
    program->addShader(vertShader);
    program->addShader(fragShader);

    stateset->setAttribute(program,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    quadGeode->setStateSet( stateset.get() );

    return quadGeode;
}

osg::Geode* DepthOfField::createDepthOfFieldQuad( osg::Texture2D* texture,osg::Texture2D* blurred )
{
    double xMin = 0;
    double xMax = _maxWidth;
    double yMin = 0;
    double yMax = _maxHeight;
    double zVal = 0;
    
    osg::Geode* quadGeode = new osg::Geode();
    osg::ref_ptr< osg::Geometry > quadGeometry = new osg::Geometry();

    osg::ref_ptr< osg::Vec3Array > quadVertices = new osg::Vec3Array();
    quadVertices->resize( 4 );
    (*quadVertices)[ 0 ].set( xMin, yMin, zVal );
    (*quadVertices)[ 1 ].set( xMax, yMin, zVal );
    (*quadVertices)[ 2 ].set( xMax, yMax, zVal );
    (*quadVertices)[ 3 ].set( xMin, yMax, zVal );
    quadGeometry->setVertexArray( quadVertices.get() );
    
    osg::ref_ptr< osg::Vec2Array > quadTexCoords = new osg::Vec2Array();
    quadTexCoords->resize( 4 );
    (*quadTexCoords)[ 0 ].set( 0, 0 );
    (*quadTexCoords)[ 1 ].set( 1, 0 );
    (*quadTexCoords)[ 2 ].set( 1, 1 );
    (*quadTexCoords)[ 3 ].set( 0, 1 );
    quadGeometry->setTexCoordArray( 0, quadTexCoords.get() );

    osg::ref_ptr< osg::Vec4Array > color = new osg::Vec4Array();
    color->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    quadGeometry->setColorArray( color.get() );
    quadGeometry->setColorBinding( osg::Geometry::BIND_OVERALL );

    quadGeometry->addPrimitiveSet( new osg::DrawArrays(
        osg::PrimitiveSet::QUADS, 0, 4 ) );
    
    quadGeode->addDrawable( quadGeometry.get() );
    quadGeode->setCullingActive( false );

    osg::ref_ptr< osg::StateSet > stateset = new osg::StateSet();
    stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    stateset->setTextureAttributeAndModes(
        0, texture, osg::StateAttribute::ON );
    stateset->setTextureAttributeAndModes(
        1, blurred, osg::StateAttribute::OFF );

    stateset->addUniform(new osg::Uniform("texture",0));
     stateset->addUniform(new osg::Uniform("blurred",1));
    
    std::string vertsource =
        "uniform vec4 offset[6]; \n"
        "void main() \n"
            "{ \n"
                "gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; \n"
                "gl_TexCoord[0] = gl_MultiTexCoord0; \n"
                //HACK, Isaac says offset the tex coord based on the window size.
                //  otherwise blur is shifted.
                "gl_TexCoord[1] = gl_MultiTexCoord0+vec4(-2./1280.,0,0,0); \n"
            "} \n";
    std::string fragsource = 
        "uniform sampler2D texture; \n" // Focused image.
        "uniform sampler2D blurred; \n" // Blurred (x and y) image.
        "void main() \n"
        "{ \n"
            // The alpha chanel of the focused image contains the normalized
            // distance from the focal point in the range 0 to 1. Use this
            // to mix color values from the blurred image with the focused image.
            "vec4 vBlurred = texture2D(blurred,gl_TexCoord[1]); \n"
            "vec4 vFullRes = texture2D(texture,gl_TexCoord[0]); \n"
            "gl_FragColor = mix(vFullRes,vBlurred,vFullRes.a); \n"
        "} \n";
    osg::Shader* vertShader = new osg::Shader();
    vertShader->setType(osg::Shader::VERTEX);
    vertShader->setShaderSource(vertsource);

    osg::Shader* fragShader = new osg::Shader();
    fragShader->setType(osg::Shader::FRAGMENT);
    fragShader->setShaderSource(fragsource);

    osg::Program* program = new osg::Program();
    program->addShader(vertShader);
    program->addShader(fragShader);

    stateset->setAttribute(program,osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    quadGeode->setStateSet( stateset.get() );

    return quadGeode;
}

osg::Program* DepthOfField::createShader()
{
    // Shaders for initially rendering the image.
    // Vertex shader is simple diffuse lighting, plus computation of
    // the normalized distance from the focal point.
    std::string vertsource = 
        "uniform float focalDist; \n"
        "uniform float focalRange; \n"
        "varying float blur; \n"
        "varying float LightIntensity; \n"
        "void main() \n"
            "{ \n"
                "vec3 ecPosition = vec3(gl_ModelViewMatrix * gl_Vertex); \n"
                "vec3 tnorm = normalize(gl_NormalMatrix * gl_Normal); \n"
                "vec3 lightVec = normalize(gl_LightSource[0].position.xyz-ecPosition); \n"
                "LightIntensity = max(dot(lightVec,tnorm),0.0); \n"

                // Use ABS of distance from focal point, we don't care if the
                // distance difference is negative. Just want absolute distance.
                // Clamp to range 0 to 1 to normalize; anything beyond 1 is "fully blurred".
                // TBD: Could use min() instead of clamp() as an optimization.
                "blur = clamp((abs(ecPosition.z -focalDist)/focalRange),0.0,1.0); \n"
                "gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; \n"
            "} \n";

    // Fragment shader stores the blur value (normalized distance
    // from focal point) in the alpha channel.
    // TBD: Add texture mapping for the cow.
    std::string fragsource = 
        "varying float blur; \n"
        "varying float LightIntensity; \n"
        "void main() \n"
        "{ \n"
        "gl_FragColor.rgb = gl_LightSource[0].diffuse.rgb * LightIntensity; \n"
        "gl_FragColor.a = blur; \n"
        //"gl_FragColor.rgb = vec3(1.,1.,1.) * blur; \n"
        "} \n";
    osg::Shader* vertShader = new osg::Shader();
    vertShader->setType(osg::Shader::VERTEX);
    vertShader->setShaderSource(vertsource);

    osg::Shader* fragShader = new osg::Shader();
    fragShader->setType(osg::Shader::FRAGMENT);
    fragShader->setShaderSource(fragsource);

    osg::Program* program = new osg::Program();
    program->addShader(vertShader);
    program->addShader(fragShader);
    return program;
}


////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    DepthOfField dof;
    osg::Group* group = new osg::Group;
    group->getOrCreateStateSet()->setAttribute(dof.createShader(),osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    std::vector< std::string > files;
    files.push_back( "dumptruck.osg" );
    files.push_back( "cow.osg.-4,-18,0.trans" );
    group->addChild( osgDB::readNodeFiles( files ) );
    if( group->getNumChildren() == 0 )
        return( 1 );

    dof.setScene(group);
    dof.setFocalPoint(-20,10);
    //dof.setWindowSize(512,512);
    dof.init( argc, argv );
    dof.getViewer()->addEventHandler(new KeyboardEventHandler(&dof));

    while (!dof.getViewer()->done())
    {
        dof.update();
        dof.getViewer()->frame();
    }
    return 0;
}
