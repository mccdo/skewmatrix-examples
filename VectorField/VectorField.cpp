//
// Copyright (c) 2008 Skew Matrix  Software LLC.
// All rights reserved.
//

#include <osgDB/FileUtils>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Texture1D>
#include <osg/Uniform>
#include <osg/ClipPlane>


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
createDataArrays( float* pos, float* dir, float* scalar )
{
    float* posI = pos;
    float* dirI = dir;
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

//
// ceilPower2 - Originally in OpenGL Distilled example source code.
//
// Return next highest power of 2 greater than x
// if x is a power of 2, return x.
unsigned short
ceilPower2( unsigned short x )
{
    if( x == 0 )
        return( 1 );

    if( (x & (x-1)) == 0 )
        // x is a power of 2.
        return( x );

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    return( x+1 );
}

void compute3DTextureSize( unsigned int dataCount, int& s, int& t, int& p )
{
    // GL_MAX_3D_TEXTURE_SIZE min value is 16.
    // NVIDIA seems to support 2048.
    s = 256;
    while( dataCount / s == 0 )
        s /= 2;

    float sliceSize( (float) dataCount / (float) s );
    float tDim = sqrtf( sliceSize );
    if( tDim > 65535.f )
        osg::notify( osg::FATAL ) << "compute3DTextureSize: Value too large " << tDim << std::endl;
    t = ceilPower2( (unsigned short)( tDim ) + 1 );

    float pDim = sliceSize / ((float)t);
    if( pDim == ((int)sliceSize) / t)
        p = ceilPower2( (unsigned short)( pDim ) );
    else
        p = ceilPower2( (unsigned short)( pDim ) + 1 );
    osg::notify( osg::ALWAYS ) << s << " " << t << " " << p << std::endl;
}

osg::Texture3D*
makeFloatTexture( int numData, unsigned char* data, int numComponents, osg::Texture::FilterMode filter )
{
    int s, t, p;
    compute3DTextureSize( numData, s, t, p );

    GLenum intFormat, pixFormat;
    if( numComponents == 1 )
    {
        intFormat = GL_ALPHA32F_ARB;
        pixFormat = GL_ALPHA;
    }
    else
    {
        // Must be 3 for now.
        intFormat = GL_RGB32F_ARB;
        pixFormat = GL_RGB;
    }
    osg::Image* image = new osg::Image;
    image->setImage( s, t, p, intFormat, pixFormat, GL_FLOAT,
        data, osg::Image::USE_NEW_DELETE );
    osg::Texture3D* texture = new osg::Texture3D( image );
    texture->setFilter( osg::Texture::MIN_FILTER, filter );
    texture->setFilter( osg::Texture::MAG_FILTER, filter );
    return( texture );
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

    /*
struct MyDrawCallback : public osg::Drawable::DrawCallback 
{
    virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const
    {
        GLint n;
        glGetIntegerv( GL_MAX_3D_TEXTURE_SIZE, &n );
        osg::notify( osg::ALWAYS ) << n << std::endl;
        drawable->drawImplementation( renderInfo );
    }
};
*/

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



    osg::ref_ptr< osg::Shader > vertexShader = new osg::Shader( osg::Shader::VERTEX );
    vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "vectorfield.vs" ) );

    osg::ref_ptr< osg::Program > program = new osg::Program();
    program->addShader( vertexShader.get() );

    osg::StateSet* ss = geode->getOrCreateStateSet();
    ss->setAttribute( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );


    float* pos( new float[ dM * dN * dO * 3 ] );
    float* dir( new float[ dM * dN * dO * 3 ] );
    float* scalar( new float[ dM * dN * dO ] );
    createDataArrays( pos, dir, scalar );

    // Posidion array.
    ss->setTextureAttribute( 0, 
        makeFloatTexture( dM*dN*dO, (unsigned char*)pos, 3, osg::Texture2D::NEAREST ) );
    osg::ref_ptr< osg::Uniform > texPosUniform =
        new osg::Uniform( "texPos", 0 );
    ss->addUniform( texPosUniform.get() );

    // Direction array.
    ss->setTextureAttribute( 1, 
        makeFloatTexture( dM*dN*dO, (unsigned char*)dir, 3, osg::Texture2D::NEAREST ) );
    osg::ref_ptr< osg::Uniform > texDirUniform =
        new osg::Uniform( "texDir", 1 );
    ss->addUniform( texDirUniform.get() );

    // Scalar array.
    ss->setTextureAttribute( 2, 
        makeFloatTexture( dM*dN*dO, (unsigned char*)scalar, 1, osg::Texture2D::NEAREST ) );
    osg::ref_ptr< osg::Uniform > texScalarUniform =
        new osg::Uniform( "scalar", 2 );
    ss->addUniform( texScalarUniform.get() );

    {
        // Pass the 3D texture dimensions to the shader as a "sizes" uniform.
        int s, t, p;
        compute3DTextureSize( dM*dN*dO, s, t, p );
        osg::ref_ptr< osg::Uniform > sizesUniform =
            new osg::Uniform( "sizes", osg::Vec3( (float)s, (float)t, (float)p ) );
        ss->addUniform( sizesUniform.get() );
    }


    // Set up the color spectrum.
    osg::Image* iColorScale = new osg::Image;
    iColorScale->setImage( 8, 1, 1, GL_RGBA, GL_RGB, GL_FLOAT,
        (unsigned char*)colorScale, osg::Image::USE_NEW_DELETE );
    osg::Texture1D* texCS = new osg::Texture1D( iColorScale );
    texCS->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::LINEAR);
    texCS->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::LINEAR );

    ss->setTextureAttribute( 3, texCS );
    osg::ref_ptr< osg::Uniform > texCSUniform =
        new osg::Uniform( "texCS", 3 );
    ss->addUniform( texCSUniform.get() );


    //delete[] pos, dir, scalar;

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

    root->getOrCreateStateSet()->addUniform( new osg::Uniform( "plane0",
        osg::Vec4( 0.707, 0.707, 0., 2. ) ) );
    root->getOrCreateStateSet()->addUniform( new osg::Uniform( "plane1",
        osg::Vec4( -0.707, -0.707, 0., 2. ) ) );

    KeyHandler* kh = new KeyHandler( uModulo.get() );

    osgViewer::Viewer viewer;
    viewer.setThreadingModel( osgViewer::ViewerBase::SingleThreaded );
    viewer.setUpViewInWindow( 10, 30, 800, 600 );
    viewer.getCamera()->setClearColor( osg::Vec4(0,0,0,0) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( kh );
    viewer.setSceneData( root.get() );
    return( viewer.run() );
}

