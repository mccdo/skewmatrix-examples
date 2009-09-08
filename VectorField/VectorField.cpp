//
// Copyright (c) 2008 Skew Matrix  Software LLC.
// All rights reserved.
//

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Geometry>
#include <osg/PositionAttitudeTransform>




const int m( 128 );
const int n( 128 );
const int nVerts( 22 );
const float dx( 1.5f ), dy( 1.5f );



void
createArrow( osg::Geometry& geom, int nInstances=1 )
{
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

    if( nInstances > 1 )
        geom.addPrimitiveSet( new osg::DrawArrays( GL_QUAD_STRIP, 0, 10, nInstances ) );
    else
        geom.addPrimitiveSet( new osg::DrawArrays( GL_QUAD_STRIP, 0, 10 ) );

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

    if( nInstances > 1 )
        geom.addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 10, 12, nInstances ) );
    else
        geom.addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLES, 10, 12 ) );
}



float*
createPositionArray( int m, int n )
{
    float* pos = new float[ m * n * 3 ];
    float* posI = pos;

    int mIdx, nIdx;
    for( mIdx = 0; mIdx < m; mIdx++ )
    {
        for( nIdx = 0; nIdx < n; nIdx++ )
        {
            *posI++ = mIdx*dx;
            *posI++ =  nIdx*dy;
            *posI++ = 0.;
        }
    }

    return pos;
}

float*
createAttitudeArray( int m, int n )
{
    float* att = new float[ m * n * 3 ];
    float* attI = att;

    int mIdx, nIdx;
    for( mIdx = 0; mIdx < m; mIdx++ )
    {
        for( nIdx = 0; nIdx < n; nIdx++ )
        {
            float nD = sqrtf( mIdx*mIdx + nIdx*nIdx ) / (float) m * 2.f;
            osg::Vec3 v( sin( -nD*osg::PI ), cos( nD*osg::PI ), sin( nD*osg::PI ) );
            if( v.length2() < .0001 )
                v.set( 0., 0., 1. );
            v.normalize();
            *attI++ = v.x();
            *attI++ = v.y();
            *attI++ = v.z();
        }
    }

    return att;
}

osg::Node*
createNonInstanced( const int m, const int n )
{
    osg::Group* grp = new osg::Group;

    osg::Geode* geode = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    createArrow( *geom, 1 );
    geode->addDrawable( geom );

    float* pos = createPositionArray( m, n );
    float* posI = pos;
    float* att = createAttitudeArray( m, n );
    float* attI = att;

    for( int iIdx=0; iIdx<m*n; iIdx++ )
    {
        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
        pat->setPosition( osg::Vec3( posI[0], posI[1], posI[2] ) );
        posI += 3;

        osg::Vec3 a( attI[0], attI[1], attI[2] );
        osg::Quat q; q.makeRotate( osg::Vec3( 0., 0., 1. ), a );
        pat->setAttitude( q );
        attI += 3;

        pat->addChild( geode );
        grp->addChild( pat );
    }

    grp->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );

    delete[] pos;
    delete[] att;

    return grp;
}

osg::Node*
createInstanced( const int m, const int n )
{
    osg::Group* grp = new osg::Group;

    osg::Geode* geode = new osg::Geode;
    osg::Geometry* geom = new osg::Geometry;
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    createArrow( *geom, m*n );
    geode->addDrawable( geom );
    grp->addChild( geode );

    osg::BoundingBox bb( 0., 0., 0., m*dx, n*dy, 1. );
    geom->setInitialBound( bb );


    std::string vertexSource =

        "uniform vec2 sizes; \n"
        "uniform sampler2D texPos; \n"
        "uniform sampler2D texAtt; \n"

        "void main() \n"
        "{ \n"
            // Using the instance ID, generate "texture coords" for this instance.
            "const float r = ((float)gl_InstanceID) / sizes.x; \n"
            "vec2 tC; \n"
            "tC.s = fract( r ); tC.t = floor( r ) / sizes.y; \n"

            // Create orthonormal basis to position and orient this instance.
            "vec4 newZ = texture2D( texAtt, tC ); \n"
            "vec3 newX = cross( newZ.xyz, vec3( 0,0,1 ) ); \n"
            "normalize( newX ); \n"
            "vec3 newY = cross( newZ.xyz, newX ); \n"
            "normalize( newY ); \n"
            "vec4 pos = texture2D( texPos, tC ); \n"
            "mat4 mV = mat4( newX.x, newX.y, newX.z, 0., newY.x, newY.y, newY.z, 0., newZ.x, newZ.y, newZ.z, 0., pos.x, pos.y, pos.z, 1. ); \n"
            "gl_Position = (gl_ModelViewProjectionMatrix * mV * gl_Vertex); \n"

            // Use just the orientation components to transform the normal.
            "mat3 mN = mat3( newX, newY, newZ ); \n"
            "vec3 norm = normalize(gl_NormalMatrix * mN * gl_Normal); \n"

            // Diffuse lighting with light at the eyepoint.
            "gl_FrontColor = gl_Color * dot( norm, vec3( 0, 0, 1 ) ); \n"

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
        new osg::Uniform( "sizes", osg::Vec2( (float)m, (float)n ) );
    ss->addUniform( sizesUniform.get() );


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

    //delete[] pos;


    float* att = createAttitudeArray( m, n );

    osg::Image* iAtt = new osg::Image;
    iAtt->setImage( m, n, 1, GL_RGB32F_ARB, GL_RGB, GL_FLOAT,
        (unsigned char*)att, osg::Image::USE_NEW_DELETE );
    osg::Texture2D* texAtt = new osg::Texture2D( iAtt );
    texAtt->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST );
    texAtt->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST );

    ss->setTextureAttribute( 1, texAtt );

    osg::ref_ptr< osg::Uniform > texAttUniform =
        new osg::Uniform( "texAtt", 1 );
    ss->addUniform( texAttUniform.get() );

    //delete[] att;

    return grp;
}

int
main( int argc,
      char ** argv )
{
    osg::ref_ptr< osg::Node > root;

    osg::notify( osg::ALWAYS ) << m*n << " instances." << std::endl;
    osg::notify( osg::ALWAYS ) << m*n*nVerts << " total vertices." << std::endl;

    //root = createNonInstanced( m, n );
    root = createInstanced( m, n );

    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.setUpViewOnSingleScreen( 0 );
    viewer.setSceneData( root.get() );
    return( viewer.run() );
}

