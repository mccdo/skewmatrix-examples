//
// Copyright (c) 2009 Skew Matrix Software LLC.
// All rights reserved.
//

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osg/Geometry>
#include <osg/io_utils>
#include <osg/math>

#include <sstream>
#include <math.h>


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





// ceilPower2
// Return next highest power of 2 greater than x
//   if x is a power of 2, return the -next- highest power of 2.
unsigned short ceilPower2( unsigned short x )
{
    if (x == 0)
        return 1;

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    return x+1;
}

// This function is used for demo purposes only. It creates the "model" file,
// just a simple planar mesh, and also creates the warp data file consisting
// of per-vertex offset vectors and normals.
void makeDataSet()
{
    const float minX( -10.f );
    const float maxX( 20.f );
    const float minY( -4.f );
    const float maxY( 3.f );
    const float z( 0.f );
    const float approxX( .5f );
    const float approxY( .25f );
    const float maxXWarp( -12.f );
    const float maxYWarp( -6.f );
    const float maxZWarp( 6.f );

    float countX( (maxX-minX) / approxX + 1.f );
    float countY( (maxY-minY) / approxY + 1.f );

    // Dimensions of the textures.
    unsigned int s = ceilPower2( (unsigned short)( countX ) );
    unsigned int t = ceilPower2( (unsigned short)( countY ) );

    osg::Vec3Array* v = new osg::Vec3Array;
    osg::ref_ptr< osg::Vec3Array> vDest = new osg::Vec3Array;
    osg::Vec3Array* n = new osg::Vec3Array;
    osg::Vec2Array* tc = new osg::Vec2Array;

    osg::BoundingBox bb;
    const osg::Vec3 normal( 0., 0., 1. );
    int xIdx, yIdx;
    for( yIdx=0; yIdx<countY; yIdx++ )
    {
        for( xIdx=0; xIdx<countX; xIdx++ )
        {
            // Store the starting mesh xyz vertices in 'v' and the starting
            // normal in 'n'.
            float xVal( (xIdx*approxX)+minX );
            float yVal( (yIdx*approxY)+minY );
            osg::Vec3 startVec( xVal, yVal, z );
            v->push_back( startVec );
            n->push_back( normal );
            osg::Vec2 coord( (float)xIdx/(float)s, (float)yIdx/(float)t );
            tc->push_back( coord );

            // Compute the destination xyz vertices and store in vDest.
            const float percent( (xVal-minX)/(maxX-minX) );
            float xOff( percent * maxXWarp );
            float yOff( percent * maxYWarp );
            float zOff = sinf( (percent * 6.f) - 3.f ) * maxZWarp;
            osg::Vec3 destVec( xVal+xOff, yVal+yOff, z+zOff );
            bb.expandBy( destVec );
            vDest->push_back( destVec );
        }
    }

    // Compute the destination normals at each vertex from the
    // destinateion vertex array vDest.
    osg::ref_ptr< osg::Vec3Array > nDest = new osg::Vec3Array;
    nDest->resize( n->size() );
    for( yIdx=0; yIdx<countY; yIdx++ )
    {
        for( xIdx=0; xIdx<countX; xIdx++ )
        {
            unsigned int idx0, idx1, idx2;
            if( ( xIdx == (countX-1) ) && ( yIdx == (countY-1) ) )
            {
                idx0 = ((yIdx-1) * countX) + (xIdx-1);
                idx1 = ((yIdx-1) * countX) + xIdx;
                idx2 = (yIdx * countX) + (xIdx-1);
            }
            else if( xIdx == (countX-1) )
            {
                idx0 = (yIdx * countX) + (xIdx-1);
                idx1 = (yIdx * countX) + xIdx;
                idx2 = ((yIdx+1) * countX) + (xIdx-1);
            }
            else if( yIdx == (countY-1) )
            {
                idx0 = ((yIdx-1) * countX) + xIdx;
                idx1 = ((yIdx-1) * countX) + (xIdx+1);
                idx2 = (yIdx * countX) + xIdx;
            }
            else
            {
                idx0 = (yIdx * countX) + xIdx;
                idx1 = (yIdx * countX) + (xIdx+1);
                idx2 = ((yIdx+1) * countX) + xIdx;
            }
            osg::Vec3 a( (*vDest)[ idx1 ] - (*vDest)[ idx0 ] );
            osg::Vec3 b( (*vDest)[ idx2 ] - (*vDest)[ idx0 ] );
            osg::Vec3 n( a ^ b );
            n.normalize();
            (*nDest)[ (yIdx * countX) + xIdx ] = n;
        }
    }

    osg::ref_ptr< osg::Geometry > geom = new osg::Geometry;
    geom->setVertexArray( v );
    geom->setNormalArray( n );
    geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
    geom->setTexCoordArray( 0, tc );
    geom->setInitialBound( bb );

    osg::Vec4Array* c = new osg::Vec4Array;
    c->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    geom->setColorArray( c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    // Compute the indices and store in a DrawElementsUInt.
    for( yIdx=0; yIdx<(countY-1); yIdx++ )
    {
        osg::ref_ptr< osg::DrawElementsUInt > deui = new osg::DrawElementsUInt( GL_TRIANGLE_STRIP, countX*2 );
        int stIdxA( (yIdx+1)*countX );
        int stIdxB( yIdx*countX );
        for( xIdx=0; xIdx<countX; xIdx++ )
        {
            (*deui)[ xIdx*2 ] = ( (unsigned int)( stIdxA+xIdx ) );
            (*deui)[ xIdx*2 + 1 ] = ( (unsigned int)( stIdxB+xIdx ) );
        }
        geom->addPrimitiveSet( deui.get() );
    }

    osg::ref_ptr< osg::Geode > geode = new osg::Geode;
    geode->addDrawable( geom.get() );
    osgDB::writeNodeFile( *geode, "warpModel.osg" );


    // Compute the difference between vDest and v. There are the offset vectors.
    osg::ref_ptr< osg::Vec3Array > vecs = new osg::Vec3Array;
    vecs->resize( s * t );
    osg::ref_ptr< osg::Vec3Array > norms = new osg::Vec3Array;
    norms->resize( s * t );
    unsigned int idx( 0 );
    for( yIdx=0; yIdx<t; yIdx++ )
    {
        for( xIdx=0; xIdx<s; xIdx++ )
        {
            if( (xIdx >= countX) || (yIdx >= countY) )
            {
                (*vecs)[ idx ].set( 0., 0., 0. );
                (*norms)[ idx ].set( 0., 0., 1. );
            }
            else
            {
                unsigned int index( (yIdx * countX) + xIdx );
                (*vecs)[ idx ].set( (*vDest)[ index ] - (*v)[ index ] );
                (*norms)[ idx ].set( (*nDest)[ index ] - (*n)[ index ] );
            }
            idx++;
        }
    }

    geom = new osg::Geometry;
    geom->setVertexArray( vecs );
    geom->setNormalArray( norms );
    geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

    // Encode the data width and height in the Geometry object name.
    std::ostringstream ostr;
    ostr << s << " " << t;
    geom->setName( ostr.str() );

    osgDB::writeObjectFile( *geom, "warpData.osg" );
}

osg::Node*
createWarp()
{
    // Load the warp data. This is an OSG Geometry with vertex offset
    // vectors stored in the VertexArray, and normal delta vectors
    // stored in the NormalArray.
    //
    // Real data will (of course) be in a different format.
    osg::Object* obj = osgDB::readObjectFile( "warpData.osg" );
    osg::Geometry* wGeom = dynamic_cast< osg::Geometry* >( obj );
    if( wGeom == NULL )
    {
        osg::notify( osg::ALWAYS ) << "Unable to load warp data." << std::endl;
        exit( 1 );
    }
    osg::Vec3Array* wVecs = dynamic_cast< osg::Vec3Array* >( wGeom->getVertexArray() );
    osg::Vec3Array* wNorms = dynamic_cast< osg::Vec3Array* >( wGeom->getNormalArray() );
    if( ( wVecs == NULL ) || ( wNorms == NULL ) )
    {
        osg::notify( osg::ALWAYS ) << "Unable to find warp offset data in warp data file." << std::endl;
        exit( 1 );
    }
    // Get the data array size. It's stored in the object name
    // in the format "<s> <t>".
    std::istringstream istr( wGeom->getName() );
    unsigned int s, t;
    istr >> s >> t;


    // Load the model file to be warped. Configure its
    // StateSet for warping.
    osg::Node* node = osgDB::readNodeFile( "warpModel.osg" );
    if( node == NULL )
    {
        osg::notify( osg::FATAL ) << "Can't load warp model file." << std::endl;
        exit( 1 );
    }

    // TBD Need to compute the initial bound by applying the vertex offsets to
    // all the vertices and creating a bounding sphere around the result.
    //
    // To avoid iterating over all the vertices and offset vectors, we just fake it:
    osg::BoundingSphere bs = node->getBound();
    bs.radius() *= 2.f;
    node->setInitialBound( bs );

    osg::StateSet* ss = node->getOrCreateStateSet();

    // specify the vector offset texture. The vertex shader will index into
    // this texture to obtain a vector to offset each xyz vertex.
    osg::Image* iVecs = new osg::Image;
    iVecs->setImage( s, t, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
        (unsigned char*) wVecs->getDataPointer(), osg::Image::USE_NEW_DELETE );
    osg::Texture2D* texVecs = new osg::Texture2D( iVecs );
    texVecs->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    texVecs->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    ss->setTextureAttribute( 0, texVecs );

    osg::ref_ptr< osg::Uniform > texVecUniform =
        new osg::Uniform( "texVec", 0 );
    ss->addUniform( texVecUniform.get() );

    // specify the normal offset texture.
    osg::Image* iNorms = new osg::Image;
    iNorms->setImage( s, t, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
        (unsigned char*) wNorms->getDataPointer(), osg::Image::USE_NEW_DELETE );
    osg::Texture2D* texNorms = new osg::Texture2D( iNorms );
    texNorms->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    texNorms->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    ss->setTextureAttribute( 1, texNorms );

    osg::ref_ptr< osg::Uniform > texNormUniform =
        new osg::Uniform( "texNorm", 1 );
    ss->addUniform( texNormUniform.get() );

    // Vertex shader to reference the vertex offset and normal offset data,
    // scale them according to elapsed time, and apply them to the incoming
    // xyz vertex and normal data.
    std::string vertexSource =

        // Vertex and normal offset textures.
        "uniform sampler2D texVec; \n"
        "uniform sampler2D texNorm; \n"

        "uniform float osg_SimulationTime; \n"

        "void main() \n"
        "{ \n"
            "float scalar = mod( osg_SimulationTime, 4. ) * .25; \n"

            // Use the current st texture coordinate to obtain the vertex and normal offset
            // for the current vertex and normal being processed.
            "vec4 vecOff = scalar * texture2D( texVec, gl_MultiTexCoord0.st ); \n"
            "vec4 normOff = scalar * texture2D( texNorm, gl_MultiTexCoord0.st ); \n"
            "vec4 position = vec4( (gl_Vertex.xyz + vecOff.xyz), gl_Vertex.w ); \n"
            "vec3 normal = normalize( gl_Normal + normOff.xyz ); \n"

            "gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * position; \n"

            "vec3 norm = gl_NormalMatrix * normal; \n"
            // Simple diffuse lighting computation with light at infinite viewer.
            "float diff = max( 0., dot( norm.xyz, vec3( 0., 0., 1. ) ) ); \n"
            "gl_FrontColor = vec4( .7*diff, .55*diff, .15*diff, 1. ); \n"

        "} \n";

    osg::ref_ptr< osg::Shader > vertexShader = new osg::Shader();
    vertexShader->setType( osg::Shader::VERTEX );
    vertexShader->setShaderSource( vertexSource );

    osg::ref_ptr< osg::Program > program = new osg::Program();
    program->addShader( vertexShader.get() );
    ss->setAttribute( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    return( node );
}


int
main( int argc,
      char ** argv )
{
    // Just do this once, to create the "model" and data files.
    //makeDataSet();
    //return( 0 );


    osgViewer::Viewer viewer;
    viewer.setSceneData( createWarp() );

    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.setUpViewOnSingleScreen( 0 );

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

