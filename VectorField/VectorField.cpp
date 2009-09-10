//
// Copyright (c) 2008 Skew Matrix  Software LLC.
// All rights reserved.
//

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Texture1D>
#include <osg/Uniform>


// (Some of the) GL 3 enums not defined by OSG.
#define GL_CLIP_DISTANCE0                 0x3000
#define GL_MAX_CLIP_DISTANCES             0x0D32


class KeyHandler : public osgGA::GUIEventHandler
{
public:
    KeyHandler( osg::Uniform* modulo )
      : _modulo( modulo )
    {}

    virtual bool handle( const osgGA::GUIEventAdapter & event_adaptor, osgGA::GUIActionAdapter & action_adaptor )
    {
        bool handled = false;
        switch( event_adaptor.getEventType() )
        {
            case ( osgGA::GUIEventAdapter::KEYDOWN ):
            {
                int key = event_adaptor.getKey();
                int keyv( key - '0' );
                if( (keyv > 0) && (keyv < 10) )
                {
                    _modulo->set( (float)keyv );
                    handled = true;
                }
                /*
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
                */
            }
        }
        return( handled );
    }

private:
    osg::ref_ptr< osg::Uniform > _modulo;
};




const int dM( 32 );
const int dN( 32 );
const int dO( 32 );

const int nVerts( 22 );
const float dx( 1.5f ), dy( 1.5f );



void
createArrow( osg::Geometry& geom, int nInstances=1 )
{
    // Create an arrow pointing in the +z direction.
    const float sD( .05 ); // shaft diameter
    const float hD( .075 ); // head diameter
    const float len( 1. ); // length
    const float sh( .65 ); // length from base to start of head

    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize( 22 );
    geom.setVertexArray( v );

    osg::Vec3Array* n = new osg::Vec3Array;
    n->resize( 22 );
    geom.setNormalArray( n );
    geom.setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

    // Shaft
    (*v)[ 0 ] = osg::Vec3( sD, 0., 0. );
    (*v)[ 1 ] = osg::Vec3( sD, 0., sh );
    (*v)[ 2 ] = osg::Vec3( 0., -sD, 0. );
    (*v)[ 3 ] = osg::Vec3( 0., -sD, sh );
    (*v)[ 4 ] = osg::Vec3( -sD, 0., 0. );
    (*v)[ 5 ] = osg::Vec3( -sD, 0., sh );
    (*v)[ 6 ] = osg::Vec3( 0., sD, 0. );
    (*v)[ 7 ] = osg::Vec3( 0., sD, sh );
    (*v)[ 8 ] = osg::Vec3( sD, 0., 0. );
    (*v)[ 9 ] = osg::Vec3( sD, 0., sh );

    (*n)[ 0 ] = osg::Vec3( 1., 0., 0. );
    (*n)[ 1 ] = osg::Vec3( 1., 0., 0. );
    (*n)[ 2 ] = osg::Vec3( 0., -1., 0. );
    (*n)[ 3 ] = osg::Vec3( 0., -1., 0. );
    (*n)[ 4 ] = osg::Vec3( -1., 0., 0. );
    (*n)[ 5 ] = osg::Vec3( -1., 0., 0. );
    (*n)[ 6 ] = osg::Vec3( 0., 1., 0. );
    (*n)[ 7 ] = osg::Vec3( 0., 1., 0. );
    (*n)[ 8 ] = osg::Vec3( 1., 0., 0. );
    (*n)[ 9 ] = osg::Vec3( 1., 0., 0. );

    // TBD tri strip
    geom.addPrimitiveSet( new osg::DrawArrays( GL_QUAD_STRIP, 0, 10, nInstances ) );

    // Head
    (*v)[ 10 ] = osg::Vec3( hD, -hD, sh );
    (*v)[ 11 ] = osg::Vec3( hD, hD, sh );
    (*v)[ 12 ] = osg::Vec3( 0., 0., len );
    osg::Vec3 norm = ((*v)[ 11 ] - (*v)[ 10 ]) ^ ((*v)[ 12 ] - (*v)[ 10 ]);
    norm.normalize();
    (*n)[ 10 ] = norm;
    (*n)[ 11 ] = norm;
    (*n)[ 12 ] = norm;

    (*v)[ 13 ] = osg::Vec3( hD, hD, sh );
    (*v)[ 14 ] = osg::Vec3( -hD, hD, sh );
    (*v)[ 15 ] = osg::Vec3( 0., 0., len );
    norm = ((*v)[ 14 ] - (*v)[ 13 ]) ^ ((*v)[ 15 ] - (*v)[ 13 ]);
    norm.normalize();
    (*n)[ 13 ] = norm;
    (*n)[ 14 ] = norm;
    (*n)[ 15 ] = norm;

    (*v)[ 16 ] = osg::Vec3( -hD, hD, sh );
    (*v)[ 17 ] = osg::Vec3( -hD, -hD, sh );
    (*v)[ 18 ] = osg::Vec3( 0., 0., len );
    norm = ((*v)[ 17 ] - (*v)[ 16 ]) ^ ((*v)[ 18 ] - (*v)[ 16 ]);
    norm.normalize();
    (*n)[ 16 ] = norm;
    (*n)[ 17 ] = norm;
    (*n)[ 18 ] = norm;

    (*v)[ 19 ] = osg::Vec3( -hD, -hD, sh );
    (*v)[ 20 ] = osg::Vec3( hD, -hD, sh );
    (*v)[ 21 ] = osg::Vec3( 0., 0., len );
    norm = ((*v)[ 20 ] - (*v)[ 19 ]) ^ ((*v)[ 21 ] - (*v)[ 19 ]);
    norm.normalize();
    (*n)[ 19 ] = norm;
    (*n)[ 20 ] = norm;
    (*n)[ 21 ] = norm;

    geom.addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 10, 12, nInstances ) );
}


void
getPosition( int m, int n, int o, float& x, float& y, float& z )
{
    const float center( 15.5f );
    x = ( m - center );
    y = ( n - center );
    z = ( o - center );
}

void
createDataArrays( float* pos, float* dir, float* cross, float* scalar )
{
    float* posI = pos;
    float* dirI = dir;
    float* crossI = cross;
    float* scalarI = scalar;

    int mIdx, nIdx, oIdx;
    for( mIdx = 0; mIdx < dM; mIdx++ )
    {
        for( nIdx = 0; nIdx < dN; nIdx++ )
        {
            for( oIdx = 0; oIdx < dO; oIdx++ )
            {
                float x, y, z;
                getPosition( mIdx, nIdx, oIdx, x, y, z );
                *posI++ = x;
                *posI++ = y;
                *posI++ = z;

                float yzLen( sqrtf( y*y + z*z ) );
                *crossI++ = 0.f;
                *crossI++ = -z/yzLen;
                *crossI++ = y/yzLen;

                *scalarI++ = yzLen / 21.9f;

                float xD;
                if( yzLen < 1.f )
                    xD = 25.f;
                else
                    xD = 3.f/yzLen;
                float yD = y * -0.1f;
                float zD = z * -0.1f;
                float len( sqrtf( xD*xD + yD*yD + zD*zD ) );
                *dirI++ = xD/len;
                *dirI++ = yD/len;
                *dirI++ = zD/len;
            }
        }
    }
}

float colorScale[] = {
    1.0f, 1.0f, 1.0f, // white
    1.0f, 0.0f, 0.0f, // red
    1.0f, 0.5f, 0.0f, // orange
    0.8f, 0.8f, 0.0f, // yellow
    0.0f, 0.8f, 0.0f, // green
    0.0f, 0.8f, 1.0f, // turquise
    0.2f, 0.2f, 1.0f, // blue
    0.5f, 0.0f, 0.7f }; // violet


osg::Node*
createInstanced()
{
    osg::Group* grp = new osg::Group;

    osg::Geode* geode = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    createArrow( *geom, dM*dN*dO );
    geode->addDrawable( geom );
    grp->addChild( geode );

    float x0, y0, z0;
    getPosition( 0, 0, 0, x0, y0, z0 );
    float x1, y1, z1;
    getPosition( dM, dN, dO, x1, y1, z1 );
    osg::BoundingBox bb( x0, y0, z0, x1, y1, z1 );
    geom->setInitialBound( bb );


    std::string vertexSource =

        "uniform vec3 sizes; \n"
        "uniform sampler3D texPos; \n"
        "uniform sampler3D texDir; \n"
        "uniform sampler3D texCross; \n"
        "uniform sampler3D scalar; \n"
        "uniform sampler1D texCS; \n"
        "uniform float modulo; \n"

        "void main() \n"
        "{ \n"
            "float fiid = gl_InstanceID; \n"

            "if( mod( fiid, modulo ) > 0.0 ) \n"
            "{ \n"
                // Discard this instance.
                // There is no straightforward way to "discard" a vertex in a vertex shader,
                // (unlike discard in a fragment shader). So we use an aspect of clipping to
                // "clip away" unwanted vertices and vectors. Here's how it works:
                // The gl_Position output of the vertex shader is an xyzw value in clip coordinates,
                // with -w < xyz < w. All xyz outside the range -w to w are clipped by hardware
                // (they are outside the view volume). So, to discard an entire vector, we set all
                // its gl_Positions to (1,1,1,0). All vertices are clipped because -0 < 1 < 0 is false.
                // If all vertices for a given instance have this value, the entire instance is
                // effectively discarded.
                "gl_Position = vec4( 1.0, 1.0, 1.0, 0.0 ); \n"
            "} \n"
            "else \n"
            "{ \n"
                // Using the instance ID, generate stp texture coords for this instance.
                "float p1 = fiid / (sizes.x*sizes.y); \n"
                "float t1 = fract( p1 ) * sizes.y;\n"
                "vec3 tC; \n"
                "tC.s = fract( t1 ); \n"
                "tC.t = floor( t1 ) / sizes.y; \n"
                "tC.p = floor( p1 ) / sizes.z; \n"

                // Sample (look up) position and orientation values.
                "vec4 pos = texture3D( texPos, tC ); \n"
                "vec4 dir = texture3D( texDir, tC ); \n"
                "vec4 c = texture3D( texCross, tC ); \n"

                // Orient the arrow.
                "vec3 up = cross( c.xyz, dir.xyz ); \n"
                "vec3 xV = vec3( c.x, up.x, dir.x ); \n"
                "vec3 yV = vec3( c.y, up.y, dir.y ); \n"
                "vec3 zV = vec3( c.z, up.z, dir.z ); \n"
                "vec4 oVec; \n"
                "oVec.x = dot( xV, gl_Vertex.xyz ); \n"
                "oVec.y = dot( yV, gl_Vertex.xyz ); \n"
                "oVec.z = dot( zV, gl_Vertex.xyz ); \n"

                // Position the oriented arrow and convert to clip coords.
                "oVec = oVec + pos; \n"
                "oVec.w = 1.0; \n"
                "gl_Position = (gl_ModelViewProjectionMatrix * oVec); \n"

                // Orient the normal.
                "vec3 oNorm; \n"
                "oNorm.x = dot( xV, gl_Normal ); \n"
                "oNorm.y = dot( yV, gl_Normal ); \n"
                "oNorm.z = dot( zV, gl_Normal ); \n"
                "vec3 norm = normalize(gl_NormalMatrix * oNorm); \n"

                // Diffuse lighting with light at the eyepoint.
                "vec4 scalarV = texture3D( scalar, tC ); \n"
                "vec4 oColor = texture1D( texCS, scalarV.a ); \n"
                "vec4 amb = oColor * 0.3; \n"
                "vec4 diff = oColor * dot( norm, vec3( 0.0, 0.0, 1.0 ) ) * 0.7; \n"
                "gl_FrontColor = amb + diff; \n"
            "} \n"
        "} \n";

    osg::ref_ptr< osg::Shader > vertexShader = new osg::Shader();
    vertexShader->setType( osg::Shader::VERTEX );
    vertexShader->setShaderSource( vertexSource );

    osg::ref_ptr< osg::Program > program = new osg::Program();
    program->addShader( vertexShader.get() );

    osg::StateSet* ss = geode->getOrCreateStateSet();
    ss->setAttribute( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    osg::ref_ptr< osg::Uniform > sizesUniform =
        new osg::Uniform( "sizes", osg::Vec3( (float)dM, (float)dN, (float)dO ) );
    ss->addUniform( sizesUniform.get() );



    float* pos( new float[ dM * dN * dO * 3 ] );
    float* dir( new float[ dM * dN * dO * 3 ] );
    float* cross( new float[ dM * dN * dO * 3 ] );
    float* scalar( new float[ dM * dN * dO ] );
    createDataArrays( pos, dir, cross, scalar );

    osg::Image* iPos = new osg::Image;
    iPos->setImage( dM, dN, dO, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
        (unsigned char*) pos, osg::Image::USE_NEW_DELETE );
    osg::Texture3D* texPos = new osg::Texture3D( iPos );
    texPos->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::NEAREST );
    texPos->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::NEAREST );

    ss->setTextureAttribute( 0, texPos );

    osg::ref_ptr< osg::Uniform > texPosUniform =
        new osg::Uniform( "texPos", 0 );
    ss->addUniform( texPosUniform.get() );



    osg::Image* iDir = new osg::Image;
    iDir->setImage( dM, dN, dO, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
        (unsigned char*)dir, osg::Image::USE_NEW_DELETE );
    osg::Texture3D* texDir = new osg::Texture3D( iDir );
    texDir->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::NEAREST );
    texDir->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::NEAREST );

    ss->setTextureAttribute( 1, texDir );

    osg::ref_ptr< osg::Uniform > texDirUniform =
        new osg::Uniform( "texDir", 1 );
    ss->addUniform( texDirUniform.get() );



    osg::Image* iCross = new osg::Image;
    iCross->setImage( dM, dN, dO, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
        (unsigned char*)cross, osg::Image::USE_NEW_DELETE );
    osg::Texture3D* texCross = new osg::Texture3D( iCross );
    texCross->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::NEAREST );
    texCross->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::NEAREST );

    ss->setTextureAttribute( 2, texCross );

    osg::ref_ptr< osg::Uniform > texCrossUniform =
        new osg::Uniform( "texCross", 2 );
    ss->addUniform( texCrossUniform.get() );



    osg::Image* iScalar = new osg::Image;
    iScalar->setImage( dM, dN, dO, GL_ALPHA32F_ARB, GL_ALPHA, GL_FLOAT,
        (unsigned char*)scalar, osg::Image::USE_NEW_DELETE );
    osg::Texture3D* texScalar = new osg::Texture3D( iScalar );
    texScalar->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::NEAREST );
    texScalar->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::NEAREST );

    ss->setTextureAttribute( 3, texScalar );

    osg::ref_ptr< osg::Uniform > texScalarUniform =
        new osg::Uniform( "scalar", 3 );
    ss->addUniform( texScalarUniform.get() );



    osg::Image* iColorScale = new osg::Image;
    iColorScale->setImage( 8, 1, 1, GL_RGBA, GL_RGB, GL_FLOAT,
        (unsigned char*)colorScale, osg::Image::USE_NEW_DELETE );
    osg::Texture1D* texCS = new osg::Texture1D( iColorScale );
    texCS->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::LINEAR);
    texCS->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::LINEAR );

    ss->setTextureAttribute( 4, texCS );

    osg::ref_ptr< osg::Uniform > texCSUniform =
        new osg::Uniform( "texCS", 4 );
    ss->addUniform( texCSUniform.get() );


    //delete[] pos, dir, cross, scalar;

    return grp;
}

int
main( int argc,
      char ** argv )
{
    osg::notify( osg::ALWAYS ) << dM*dN*dO << " instances." << std::endl;
    osg::notify( osg::ALWAYS ) << dM*dN*dO*nVerts << " total vertices." << std::endl;

    osg::ref_ptr< osg::Node > root = createInstanced();

    osg::ref_ptr< osg::Uniform > uModulo( new osg::Uniform( "modulo", 1.0f ) );
    uModulo->setDataVariance( osg::Object::DYNAMIC );
    root->getOrCreateStateSet()->addUniform( uModulo.get() );

    KeyHandler* kh = new KeyHandler( uModulo.get() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 600 );
    viewer.getCamera()->setClearColor( osg::Vec4(0,0,0,0) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( kh );
    viewer.setSceneData( root.get() );
    return( viewer.run() );
}

