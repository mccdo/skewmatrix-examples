uniform vec3 sizes;
uniform sampler3D texPos;
uniform sampler3D texDir;
uniform sampler3D scalar;
uniform sampler1D texCS;

uniform float modulo;

uniform vec4 plane0;
uniform vec4 plane1;
uniform vec4[ 6 ] planes;
uniform int[ 6 ] planeOn;


// There is no straightforward way to "discard" a vertex in a vertex shader,
// (unlike discard in a fragment shader). So we use an aspect of clipping to
// "clip away" unwanted vertices and vectors. Here's how it works:
// The gl_Position output of the vertex shader is an xyzw value in clip coordinates,
// with -w < xyz < w. All xyz outside the range -w to w are clipped by hardware
// (they are outside the view volume). So, to discard an entire vector, we set all
// its gl_Positions to (1,1,1,0). All vertices are clipped because -0 < 1 < 0 is false.
// If all vertices for a given instance have this value, the entire instance is
// effectively discarded.
bool
discardInstance( const in float fiid )
{
    bool discardInstance = ( mod( fiid, modulo ) > 0.0 );
    if( discardInstance )
        gl_Position = vec4( 1.0, 1.0, 1.0, 0.0 );
    return( discardInstance );
}

// Based on the global 'sizes' uniform that contains the 3D stp texture dimensions,
// and the input parameter current instances, generate an stp texture coord that
// indexes into a texture to obtain data for this instance.
vec3
generateTexCoord( const in float fiid )
{
    float p1 = fiid / (sizes.x*sizes.y);
    float t1 = fract( p1 ) * sizes.y;
    
    vec3 tC;
    tC.s = fract( t1 );
    tC.t = floor( t1 ) / sizes.y;
    tC.p = floor( p1 ) / sizes.z;

    return( tC );
}

bool
clipInstance( const in vec3 pos )
{
    bool clip = false;
    int idx;
    for( idx=0; idx<6; idx++ )
    {
        if( planeOn[idx] != 0 )
        {
            if( dot( planes[ idx ], vec4( pos.xyz, 1.0 ) ) < 0.0 )
            {
                gl_Position = vec4( 1.0, 1.0, 1.0, 0.0 );
                clip = true;
                break;
            }
        }
    }
    return( clip );
}

mat3
makeOrientMat( const in vec4 dir )
{
    // Compute a vector at a right angle to the direction.
    // First try projection direction into xy rotated -90 degrees.
    // If that gives us a very short vector,
    // then project into yz instead, rotated -90 degrees.
    vec3 c = vec3( dir.y, -dir.x, 0.0 );
    if( dot( c, c ) < 0.1 )
        c = vec3( 0.0, dir.z, -dir.y );
    normalize( c );

    const vec3 up = normalize( cross( c.xyz, dir.xyz ) );

    // Orientation uses the cross product vector as x,
    // the up vector as y, and the direction vector as z.
    return( mat3( c, up, dir.xyz ) );
}

vec4
simpleLighting( const in vec4 color, const in vec3 normal, const in float diffCont, const in float ambCont )
{
    const vec4 amb = color * ambCont;
    const vec3 eyeVec = vec3( 0.0, 0.0, 1.0 );
    const float dotVal = max( dot( normal, eyeVec ), 0.0 );
    const vec4 diff = color * dotVal * diffCont;
    return( amb + diff );
}

void main()
{
    // Get instance ID and discard entire arrow if this instance is not to be rendered.
    float fiid = gl_InstanceID;
    if( discardInstance( fiid ) )
        return;

    // Generate stp texture coords from the instance ID.
    vec3 tC = generateTexCoord( fiid );

    // Sample (look up) position. Discard instance if clipped.
    vec4 pos = texture3D( texPos, tC );
    if( clipInstance( pos ) )
        return;

    // Sample (look up) direction vector and obtain the scale factor
    vec4 dir = texture3D( texDir, tC );
    float scale = length( dir );

    // Create an orientation matrix. Orient/transform the arrow.
    const mat3 orientMat = makeOrientMat( normalize( dir ) );
    const vec3 oVec = orientMat * (scale * gl_Vertex.xyz);
    vec4 hoVec = vec4( oVec + pos, 1.0 );
    gl_Position = gl_ModelViewProjectionMatrix * hoVec;

    // Orient the normal.
    const vec3 norm = normalize( gl_NormalMatrix * orientMat * gl_Normal );

    // Compute color and lighting.
#if 1
    // Scalar texture containg key to color table.
    const vec4 scalarV = texture3D( scalar, tC );
    const vec4 oColor = texture1D( texCS, scalarV.a );
#else
    // Scalat texture contains rgb values.
    const vec4 oColor = texture3D( scalar, tC );
#endif
    gl_FrontColor = simpleLighting( oColor, norm, 0.7, 0.3 );
}
