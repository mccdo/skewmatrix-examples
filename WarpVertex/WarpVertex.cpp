//
// Copyright (c) 2008 Skew Matrix  Software LLC.
// All rights reserved.
//

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

#include <osg/Geometry>
#include <osg/io_utils>
#include <osg/math>
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

void
createMeshData( osg::Geometry* geom )
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

    geom->setVertexArray( v );
    geom->setNormalArray( n );
    geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
    geom->setTexCoordArray( 0, tc );
    geom->setInitialBound( bb );

    for( yIdx=0; yIdx<(countY-1); yIdx++ )
    {
        osg::ref_ptr< osg::DrawElementsUInt > deui = new osg::DrawElementsUInt( GL_TRIANGLE_STRIP, countX*2 );
        int stIdxA( (yIdx+1)*countX );
        int stIdxB( yIdx*countX );
        for( xIdx=0; xIdx<countX; xIdx++ )
        {
            deui->push_back( (unsigned int)( stIdxA+xIdx ) );
            deui->push_back( (unsigned int)( stIdxB+xIdx ) );
        }
        geom->addPrimitiveSet( deui.get() );
    }

    osg::StateSet* ss = geom->getOrCreateStateSet();

    // Compute the difference between vDest and v. There are the offset vectors.
    float* vecs = new float[ s * t * 3 ];
    float* vecsPtr = vecs;
    float* norms = new float[ s * t * 3 ];
    float* normsPtr = norms;
    for( yIdx=0; yIdx<t; yIdx++ )
    {
        for( xIdx=0; xIdx<s; xIdx++ )
        {
            if( (xIdx >= countX) || (yIdx >= countY) )
            {
                *vecsPtr++ = 0.;
                *vecsPtr++ = 0.;
                *vecsPtr++ = 0.;
                *normsPtr++ = 0.;
                *normsPtr++ = 0.;
                *normsPtr++ = 1.;
            }
            else
            {
                unsigned int index( (yIdx * countX) + xIdx );
                osg::Vec3 vector( (*vDest)[ index ] - (*v)[ index ] );
                *vecsPtr++ = vector.x();
                *vecsPtr++ = vector.y();
                *vecsPtr++ = vector.z();
                osg::Vec3 normal( (*nDest)[ index ] - (*n)[ index ] );
                *normsPtr++ = normal.x();
                *normsPtr++ = normal.y();
                *normsPtr++ = normal.z();
            }
        }
    }

    // specify the vector offset texture. The vertex shader will index into
    // this texture to obtain a vector to offset each xyz vertex.
    osg::Image* iVecs = new osg::Image;
    iVecs->setImage( s, t, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
        (unsigned char*) vecs, osg::Image::USE_NEW_DELETE );
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
        (unsigned char*) norms, osg::Image::USE_NEW_DELETE );
    osg::Texture2D* texNorms = new osg::Texture2D( iNorms );
    texNorms->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    texNorms->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );
    ss->setTextureAttribute( 1, texNorms );

    osg::ref_ptr< osg::Uniform > texNormUniform =
        new osg::Uniform( "texNorm", 1 );
    ss->addUniform( texNormUniform.get() );

    std::string vertexSource =

        "uniform sampler2D texVec; \n"
        "uniform sampler2D texNorm; \n"

        "uniform float osg_SimulationTime; \n"

        "void main() \n"
        "{ \n"
            "float scalar = mod( osg_SimulationTime, 4. ) * .25; \n"

            "vec4 vecOff = scalar * texture2D( texVec, gl_MultiTexCoord0.st ); \n"
            "vec4 normOff = scalar * texture2D( texNorm, gl_MultiTexCoord0.st ); \n"
            "vec4 position = vec4( (gl_Vertex.xyz + vecOff.xyz), gl_Vertex.w ); \n"
            "vec3 normal = normalize( gl_Normal + normOff.xyz ); \n"

            "gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * position; \n"

            "vec3 norm = gl_NormalMatrix * normal; \n"
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
}

osg::Group*
createMesh()
{
    osg::ref_ptr< osg::Group > grp = new osg::Group;
    osg::Geode* geode = new osg::Geode;
    grp->addChild( geode );

    osg::Geometry* geom = new osg::Geometry();
    geode->addDrawable( geom );

    createMeshData( geom );

    osg::Vec4Array* c = new osg::Vec4Array;
    c->push_back( osg::Vec4( 1., 1., 1., 1. ) );
    geom->setColorArray( c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    return( grp.release() );
}

int
main( int argc,
      char ** argv )
{
    osg::ref_ptr< osg::Group > root = new osg::Group;
    root->addChild( createMesh() );

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

