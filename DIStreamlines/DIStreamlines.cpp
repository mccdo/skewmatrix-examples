//
// Copyright (c) 2008 Skew Matrix  Software LLC.
// All rights reserved.
//

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Point>
#include <osg/PointSprite>
#include <osg/AlphaFunc>


// Allows you to change the animation play rate:
//   '+' speed up
//   '-' slow down
//   'p' pause
class PlayStateHandler
    : public osgGA::GUIEventHandler
{
public:
    PlayStateHandler()
      : elapsedTime( 0. ),
        scalar( 1. ),
        paused( false )
    {}

    double getCurrentTime() const
    {
        if( paused )
            return( elapsedTime );
        else
            return( elapsedTime + ( timer.time_s() * scalar ) );
    }

    virtual bool handle( const osgGA::GUIEventAdapter & event_adaptor,
                         osgGA::GUIActionAdapter & action_adaptor )
    {
        bool handled = false;
        switch( event_adaptor.getEventType() )
        {
            case ( osgGA::GUIEventAdapter::KEYDOWN ):
            {
                int key = event_adaptor.getKey();
                switch( key )
                {
                    case '+': // speed up
                    {
                        elapsedTime = getCurrentTime();
                        timer.setStartTick( timer.tick() );

                        // Increase speed by 33%
                        scalar *= ( 4./3. );

                        handled = true;
                    }
                    break;
                    case '-': // slow down
                    {
                        elapsedTime = getCurrentTime();
                        timer.setStartTick( timer.tick() );

                        // Decrease speed by 25%
                        scalar *= .75;

                        handled = true;
                    }
                    break;
                    case 'p': // pause
                    {
                        elapsedTime = getCurrentTime();
                        timer.setStartTick( timer.tick() );

                        paused = !paused;

                        handled = true;
                    }
                    break;

                }
            }
        }
        return( handled );
    }

private:
    osg::Timer timer;
    double elapsedTime;
    double scalar;
    bool paused;
};



// m and n are the dimensions of the texture that store the position values.
// m * n is the total number of point instances that will be rendered to
// create the streamline.
const int m( 1 );
const int n( 256 );

// Distance between points. Smaller values look better at near distances,
// larger values look better at far distances. Could possible vary this
// value dynamically...
const float dX( .25f );



void
createSLPoint( osg::Geometry& geom, int nInstances, const osg::Vec3 position, const osg::Vec4 color )
{
    // Configure a Geometry to draw a single point, but use the draw instanced PrimitiveSet
    // to draw the point multiple times.
    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize( 1 );
    geom.setVertexArray( v );
    (*v)[ 0 ] = position;

    // Streamline color. Blending is non-saturating, so it never
    // reaches full intensity white. Alpha is modulated with the
    // point sprint texture alpha, so the value here is a maximum
    // for the "densist" part of the point sprint texture.
    osg::Vec4Array* c = new osg::Vec4Array;
    c->resize( 1 );
    geom.setColorArray( c );
    geom.setColorBinding( osg::Geometry::BIND_OVERALL );
    (*c)[ 0 ] = color;

    geom.addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, 1, nInstances ) );


    osg::StateSet* ss = geom.getOrCreateStateSet();

    osg::Point* point = new osg::Point;
    point->setSize( 40. );
    // Use of shader (required for draw instanced) disables fixed-funxtion point parameters.
    // I'll need to investigate how to mimic this functionality in a shader.
    //point->setDistanceAttenuation( osg::Vec3( 0., 0., 0.05f) );
    ss->setAttributeAndModes( point );

    // Turn on point sprites and specigy the point sprite texture.
    osg::PointSprite *sprite = new osg::PointSprite();
    ss->setTextureAttributeAndModes( 1, sprite, osg::StateAttribute::ON );
    osg::Texture2D *tex = new osg::Texture2D();
    tex->setImage( osgDB::readImageFile( "splotch.png" ) );
    ss->setTextureAttributeAndModes( 1, tex, osg::StateAttribute::ON );

    // Keep pixels with a significant alpha value (discard low-alpha pixels).
    osg::AlphaFunc* af = new osg::AlphaFunc( osg::AlphaFunc::GREATER, 0.05f );
    ss->setAttributeAndModes( af );
}


// Create an array of xyz float position values for each point in the streamline.
float*
createPositionArray( int m, int n )
{
    float* pos = new float[ m * n * 3 ];
    float* posI = pos;

    int iIdx;
    for( iIdx = 0; iIdx < m*n; iIdx++ )
    {
        *posI++ = iIdx*dX;
        *posI++ = 0.;
        *posI++ = 4. * sin( iIdx * .03 );
    }

    return pos;
}


// Create a scene graph and state set configured to render a streamline using draw instanced.
osg::Group*
createInstanced( const int m, const int n )
{
    // Essentially a top level Group, a single Geode child, and the
    // Geode contains a single Geometry to draw a sinalg point (but
    // uses a draw instanced PrimitiveSet).
    osg::Group* grp = new osg::Group;
    osg::Geode* geode = new osg::Geode;
    grp->addChild( geode );

    osg::Geometry* geom = new osg::Geometry;
    // Note:
    // Display Lists and draw instanced are mutually exclusive. Disable
    // display lists and use buffer objects instead.
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    osg::Vec3 loc( -3., 0., 0. );
    createSLPoint( *geom, m*n, loc, osg::Vec4( .6, .7, 1., 1. ) );
    geode->addDrawable( geom );

    // Note:
    // OSG has no idea where our vertex shader will render the points. For proper culling
    // and near/far computation, set an approximate initial bounding box.
    osg::BoundingBox bb( osg::Vec3( 0., -1., -4. )+loc, osg::Vec3( m*n*dX, 1., 4. )+loc );
    geom->setInitialBound( bb );

    // 2nd streamline
    geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    loc.set( 0., -3., 1. );
    createSLPoint( *geom, m*n, loc, osg::Vec4( 1., .7, .5, 1. ) );
    geode->addDrawable( geom );

    bb = osg::BoundingBox( osg::Vec3( 0., -1., -4. )+loc, osg::Vec3( m*n*dX, 1., 4. )+loc );
    geom->setInitialBound( bb );

    // 3rd streamline
    geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    loc.set( 2., 4.5, 1.5 );
    createSLPoint( *geom, m*n, loc, osg::Vec4( .5, 1., .6, 1. ) );
    geode->addDrawable( geom );

    bb = osg::BoundingBox( osg::Vec3( 0., -1., -4. )+loc, osg::Vec3( m*n*dX, 1., 4. )+loc );
    geom->setInitialBound( bb );


    osg::StateSet* ss = geode->getOrCreateStateSet();

    osg::ref_ptr< osg::Shader > vertexShader = osg::Shader::readShaderFile(
        osg::Shader::VERTEX, osgDB::findDataFile( "streamline.vs" ) );

    osg::ref_ptr< osg::Program > program = new osg::Program();
    program->addShader( vertexShader.get() );
    ss->setAttribute( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    // Note:
    // We will render the streamline points with depth test on and depth write disabled,
    // with an order independent blend. This means we need to draw the streamlines last
    // (so use bin # 10) but we don't need the depth sort, so use bin name "RenderBin".
    ss->setRenderBinDetails( 10, "RenderBin" );

    // Note:
    // When using a vertex shader, point size is taken from glPointSize and _not_
    // distance-attenuated. However, set the following mode ON, and then our vertex
    // shader can do its own distance attenuation and emit gl_PointSize.
    ss->setMode( GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON );

    // Tells the shader the dimensions of our texture: m x n.
    // Required to compute correct texture coordinates from the instance ID.
    osg::ref_ptr< osg::Uniform > sizesUniform =
        new osg::Uniform( "sizes", osg::Vec2( (float)m, (float)n ) );
    ss->addUniform( sizesUniform.get() );

    // Tell the shader the total number of instances: m * n.
    // Required for animation based on the instance ID.
    osg::ref_ptr< osg::Uniform > totalInstancesUniform =
        new osg::Uniform( "totalInstances", (float)(m * n) );
    ss->addUniform( totalInstancesUniform.get() );

    // Specify the time in seconds for a given streamline point to fade
    // from full intensity to zero intensity.
    // (May be altered with simulation time.)
    osg::ref_ptr< osg::Uniform > fadeTimeUniform =
        new osg::Uniform( "fadeTime", 1.f );
    ss->addUniform( fadeTimeUniform.get() );

    // Specify the time in seconds for the animation to loop.
    // (May be altered with simulation time.)
    osg::ref_ptr< osg::Uniform > repeatTimeUniform =
        new osg::Uniform( "repeatTime", 3.f );
    ss->addUniform( repeatTimeUniform.get() );

    // Note:
    // It turns out that SRC_ALPHA, ONE_MINUS_SRC_ALPHA actually is
    // non-saturating. Give it a color just shy of full intensity white,
    // and the result will never saturate to white no matter how many
    // times it is overdrawn.
    osg::ref_ptr< osg::BlendFunc > bf = new osg::BlendFunc(
        GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    ss->setAttributeAndModes( bf.get() );

    // Note:
    // Leave the depth test enabled, but mask off depth writes (4th param is false).
    // This allows us to render the streamline points in any order, front to back
    // or back to front, and not lose any points by depth testing against themselves.
    osg::ref_ptr< osg::Depth > depth = new osg::Depth( osg::Depth::LESS, 0., 1., false );
    ss->setAttributeAndModes( depth.get() );

    // Note:
    // After drawing opaque objects, translucency can be order-independent only if
    // certain criteria are met:
    // 1. The alpha values of all pixels must be the same, OR
    //    The RGB valuues of all pixels must be the same.
    // 2. Depth write must be disabled so that far translucent pixels don't lose the
    //    depth test to near translucent pixels.
    // 3. The blend function must not reference destination alpha.


    // specify the position texture. The vertex shader will index into
    // this texture to obtain position values for each streamline point.
    float* pos = createPositionArray( m, n );
    osg::Image* iPos = new osg::Image;
    iPos->setImage( m, n, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
        (unsigned char*) pos, osg::Image::USE_NEW_DELETE );
    osg::Texture2D* texPos = new osg::Texture2D( iPos );
    texPos->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    texPos->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    ss->setTextureAttribute( 0, texPos );

    osg::ref_ptr< osg::Uniform > texPosUniform =
        new osg::Uniform( "texPos", 0 );
    ss->addUniform( texPosUniform.get() );


    return grp;
}

// Make some opaque boxes to show that depth testing works properly.
osg::Group*
createOpaque()
{
    osg::ref_ptr< osg::Group > grp = new osg::Group;
    osg::Geode* geode = new osg::Geode;
    grp->addChild( geode );

    osg::Box* box = new osg::Box( osg::Vec3( 14., 0., -.5 ), 4., 3., 2. );
    osg::ShapeDrawable* shape = new osg::ShapeDrawable( box );
    shape->setColor( osg::Vec4( .2, .2, 1., 1. ) );
    geode->addDrawable( shape );

    box = new osg::Box( osg::Vec3( 36., 0., 1. ), 2., 6., 3. );
    shape = new osg::ShapeDrawable( box );
    shape->setColor( osg::Vec4( .2, .8, .2, 1. ) );
    geode->addDrawable( shape );

    return( grp.release() );
}

int
main( int argc,
      char ** argv )
{
    osg::ref_ptr< osg::Group > root = new osg::Group;
    root->addChild( createInstanced( m, n ) );
    root->addChild( createOpaque() );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.setUpViewOnSingleScreen( 0 );
    viewer.setSceneData( root.get() );

    viewer.setCameraManipulator( new osgGA::TrackballManipulator );

    // Create a PlayStateHandler to track elapsed simulation time
    // and play/pause state.
    osg::ref_ptr< PlayStateHandler > psh = new PlayStateHandler;
    viewer.addEventHandler( psh.get() );

    while (!viewer.done())
    {
        // Get time from the PlayStateHandler.
        double simTime = psh->getCurrentTime();
        viewer.frame( simTime );
    }

    return( 0 );
}

